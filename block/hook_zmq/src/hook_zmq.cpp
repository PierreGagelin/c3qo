

// Project headers
#include "block/hook_zmq.hpp"
#include "engine/manager.hpp"

hook_zmq::hook_zmq(struct manager *mgr) : block(mgr),
                                          client_(false),
                                          type_(ZMQ_PAIR),
                                          name_(""),
                                          addr_("tcp://127.0.0.1:6666"),
                                          rx_pkt_(0u),
                                          tx_pkt_(0u)
{
}

hook_zmq::~hook_zmq() {}

//
// @brief Receive a ZMQ multi-parts message
//
void hook_zmq::recv_(struct buffer &buf)
{
    int ret;

    ret = 1;
    while (ret == 1)
    {
        zmq_msg_t part;

        // Receive a message
        zmq_msg_init(&part);

        ret = zmq_msg_recv(&part, zmq_sock_.socket, ZMQ_DONTWAIT);
        if (ret <= 0)
        {
            LOGGER_ERR("Failed to receive data from ZMQ socket: %s [errno=%d]", strerror(errno), errno);
            break;
        }

        buf.push_back(zmq_msg_data(&part), zmq_msg_size(&part));

        // Look if there is another part to come
        ret = zmq_msg_more(&part);

        zmq_msg_close(&part);
    }
}

//
// @brief Send a ZMQ multi-part message
//
bool hook_zmq::send_(struct buffer &buf)
{
    int flags = ZMQ_DONTWAIT | ZMQ_SNDMORE;

    for (size_t idx = 0u; idx < buf.parts_.size(); ++idx)
    {
        zmq_msg_t message;
        int rc;

        zmq_msg_init_size(&message, buf.parts_[idx].len);
        memcpy(zmq_msg_data(&message), buf.parts_[idx].data, buf.parts_[idx].len);

        if (idx == buf.parts_.size() - 1)
        {
            flags &= ~ZMQ_SNDMORE;
        }

        rc = zmq_msg_send(&message, zmq_sock_.socket, flags);
        if (rc == -1)
        {
            LOGGER_ERR("Failed to send ZMQ message: %s [errno=%d]", strerror(errno), errno);
            zmq_msg_close(&message);
            return false;
        }
    }
    return true;
}

//
// @brief Callback to handle data available on the socket
//
void hook_zmq::on_fd_(struct file_desc &fd)
{
    struct buffer buf;

    if (fd.socket != zmq_sock_.socket)
    {
        LOGGER_ERR("Failed to receive message: unknown socket [expected=%p ; actual=%p]",
                   zmq_sock_.socket,
                   fd.socket);
        return;
    }

    // Read a possibly multi-part message
    recv_(buf);
    ++rx_pkt_;

    LOGGER_DEBUG("Received message [bk_id=%d ; parts_count=%zu]", id_, buf.parts_.size());

    // Send it to the next block
    process_data_(1, &buf);

    buf.clear();
}

//
// @brief Start the block
//
void hook_zmq::start_()
{
    int ret;

    // Create a ZMQ context
    zmq_ctx_ = zmq_ctx_new();
    if (zmq_ctx_ == nullptr)
    {
        LOGGER_ERR("Failed to create ZMQ context [bk_id=%d]", id_);
        return;
    }

    // Create the socket
    zmq_sock_.socket = zmq_socket(zmq_ctx_, type_);
    if (zmq_sock_.socket == nullptr)
    {
        LOGGER_ERR("Failed to create ZMQ socket [bk_id=%d ; type=%d]", id_, type_);
        return;
    }

    // Set identity
    ret = zmq_setsockopt(zmq_sock_.socket, ZMQ_IDENTITY, name_.c_str(), name_.size() + 1);
    if (ret != 0)
    {
        LOGGER_ERR("Failed to set ZMQ_IDENTITY [bk_id=%d ; identity=%s]", id_, name_.c_str());
        return;
    }

    // Bind or connect the socket
    if (client_ == true)
    {
        ret = zmq_connect(zmq_sock_.socket, addr_.c_str());
        if (ret == -1)
        {
            LOGGER_ERR("Failed to connect ZMQ socket: %s [errno=%d ; bk_id=%d ; addr=%s]",
                       strerror(errno),
                       errno,
                       id_,
                       addr_.c_str());
            return;
        }
        LOGGER_DEBUG("Connected client socket [bk_id=%d ; addr=%s]", id_, addr_.c_str());
    }
    else
    {
        ret = zmq_bind(zmq_sock_.socket, addr_.c_str());
        if (ret == -1)
        {
            LOGGER_ERR("Failed to bind ZMQ socket: %s [errno=%d ; bk_id=%d ; addr=%s]",
                       strerror(errno),
                       errno,
                       id_,
                       addr_.c_str());
            return;
        }
        LOGGER_DEBUG("Bound server socket [bk_id=%d ; addr=%s]", id_, addr_.c_str());
    }

    // Register a callback for reception
    zmq_sock_.bk = this;
    zmq_sock_.fd = -1;
    zmq_sock_.read = true;
    zmq_sock_.write = false;
    mgr_->fd_add(zmq_sock_);

    // Send a first message to register identity
    if (type_ == ZMQ_DEALER)
    {
        struct buffer buf;
        const char *dummy = "dummy";

        buf.push_back(dummy, strlen(dummy));

        bool is_ok = send_(buf);
        if (is_ok == false)
        {
            LOGGER_ERR("Failed to register identity");
            return;
        }

        buf.clear();
    }

    LOGGER_INFO("Started ZMQ hook [bk_id=%d ; type=%d]", id_, type_);
}

//
// @brief Stop the block
//
void hook_zmq::stop_()
{
    // Remove the socket's callback
    mgr_->fd_remove(zmq_sock_);

    // Close the socket
    zmq_close(zmq_sock_.socket);

    // Delete the context
    zmq_ctx_term(zmq_ctx_);

    LOGGER_INFO("Stopped ZMQ hook [bk_id=%d]", id_);
}

//
// @brief Send data to the exterior
//
int hook_zmq::data_(void *vdata)
{
    bool ok;

    if (vdata == nullptr)
    {
        LOGGER_ERR("Failed to process buffer: nullptr data");
        return PORT_STOP;
    }
    struct buffer &buf = *(static_cast<struct buffer *>(vdata));

    // Send topic and data
    ok = send_(buf);
    if (ok == true)
    {
        LOGGER_DEBUG("Message sent on ZMQ socket [bk_id=%d ; parts=%zu]", id_, buf.parts_.size());
        tx_pkt_++;
    }

    return PORT_STOP;
}

//
// Implementation of the factory interface
//

struct block *hook_zmq_factory::constructor(struct manager *mgr)
{
    return new struct hook_zmq(mgr);
}

void hook_zmq_factory::destructor(struct block *bk)
{
    delete static_cast<struct hook_zmq *>(bk);
}
