

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
// @brief Callback to handle data available on the socket
//
void hook_zmq::on_fd_(struct file_desc &fd)
{
    std::vector<struct c3qo_zmq_part> msg;

    if (fd.socket != zmq_sock_.socket)
    {
        LOGGER_ERR("Failed to receive message: unknown socket [expected=%p ; actual=%p]",
                   zmq_sock_.socket,
                   fd.socket);
        return;
    }

    // Read a possibly multi-part message
    socket_zmq_read(zmq_sock_.socket, msg);
    ++rx_pkt_;

    LOGGER_DEBUG("Received message [bk_id=%d ; parts_count=%zu]", id_, msg.size());

    // Send it to the next block
    process_data_(1, &msg);

    c3qo_zmq_msg_del(msg);
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

    // Register the subscriber's callback
    zmq_sock_.bk = this;
    zmq_sock_.fd = -1;
    zmq_sock_.read = true;
    zmq_sock_.write = false;
    mgr_->fd_add(zmq_sock_);

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
    std::vector<struct c3qo_zmq_part> *msg;
    bool ok;

    if (vdata == nullptr)
    {
        LOGGER_ERR("Failed to process buffer: nullptr data");
        return PORT_STOP;
    }
    msg = static_cast<std::vector<struct c3qo_zmq_part> *>(vdata);

    // Send topic and data
    ok = socket_zmq_write(zmq_sock_.socket, *msg);
    if (ok == true)
    {
        LOGGER_DEBUG("Message sent on ZMQ socket [bk_id=%d ; parts=%zu]", id_, msg->size());
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
