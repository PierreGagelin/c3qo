

// Project headers
#include "block/zmq_pair.hpp"
#include "engine/manager.hpp"

// Needle to look for in configuration
#define NEEDLE_TYPE "type=" // either "server" or "client"
#define NEEDLE_ADDR "addr=" // fully specified address

zmq_pair::zmq_pair(struct manager *mgr) : block(mgr), client_(false), addr_("tcp://127.0.0.1:6666"), rx_pkt_(0u), tx_pkt_(0u)
{
    // Create a ZMQ context
    zmq_ctx_ = zmq_ctx_new();
    ASSERT(zmq_ctx_ != nullptr);

    // Create a ZMQ socket
    zmq_sock_.socket = zmq_socket(zmq_ctx_, ZMQ_PAIR);
    ASSERT(zmq_sock_.socket != nullptr);
}

zmq_pair::~zmq_pair()
{
    // Close a ZMQ socket
    zmq_close(zmq_sock_.socket);

    // Delete a ZMQ context
    zmq_ctx_term(zmq_ctx_);
}

//
// @brief Callback to handle data available on the socket
//
void zmq_pair::on_fd_(struct file_desc &fd)
{
    std::vector<struct c3qo_zmq_part> msg;

    if (fd.socket != zmq_sock_.socket)
    {
        LOGGER_ERR("Failed to receive message: unknown socket [expected=%p ; actual=%p]", zmq_sock_.socket, fd.socket);
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

void zmq_pair::conf_(char *conf)
{
    char type[32];
    char addr[32];
    int ret;

    // Verify input
    if (conf == nullptr)
    {
        LOGGER_ERR("Failed to configure block: nullptr conf");
        return;
    }

    LOGGER_INFO("Configure block [bk_id=%d ; conf=%s]", id_, conf);

    // Retrieve type and address
    ret = sscanf(conf, NEEDLE_TYPE "%31s " NEEDLE_ADDR "%31s", type, addr);
    if (ret == EOF)
    {
        LOGGER_ERR("Failed to call sscanf: %s [errno=%d ; str=%s]", strerror(errno), errno, conf);
        return;
    }
    else if (ret != 2)
    {
        LOGGER_ERR("Failed to call sscanf: wrong number of matched element [expected=2 ; actual=%d]", ret);
        return;
    }

    if (strcmp(type, "client") == 0)
    {
        client_ = true;
    }
    else if (strcmp(type, "server") == 0)
    {
        client_ = false;
    }
    else
    {
        LOGGER_ERR("Failed to configure socket type: unknown type [bk_id=%d ; type=%s]", id_, type);
        return;
    }
    addr_ = std::string(addr);

    LOGGER_INFO("Configured ZMQ socket [type=%s ; addr=%s]", type, addr_.c_str());
}

//
// @brief Start the block
//
void zmq_pair::start_()
{
    int ret;

    LOGGER_INFO("Start block ZMQ Pair [bk_id=%d]", id_);

    // Bind or connect the socket
    if (client_ == true)
    {
        ret = zmq_connect(zmq_sock_.socket, addr_.c_str());
        if (ret == -1)
        {
            LOGGER_ERR("Failed to connect ZMQ socket: %s [bk_id=%d ; addr=%s ; errno=%d]", strerror(errno), id_, addr_.c_str(), errno);
            return;
        }
        LOGGER_DEBUG("Connected client socket [bk_id=%d ; addr=%s]", id_, addr_.c_str());
    }
    else
    {
        ret = zmq_bind(zmq_sock_.socket, addr_.c_str());
        if (ret == -1)
        {
            LOGGER_ERR("Failed to bind ZMQ socket: %s [bk_id=%d ; addr=%s ; errno=%d]", strerror(errno), id_, addr_.c_str(), errno);
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
}

//
// @brief Stop the block
//
void zmq_pair::stop_()
{
    // Remove the socket's callback
    mgr_->fd_remove(zmq_sock_);

    LOGGER_INFO("Stop block ZMQ Pair [bk_id=%d]", id_);
}

//
// @brief Send data to the exterior
//
int zmq_pair::data_(void *vdata)
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

struct block *zmq_pair_factory::constructor(struct manager *mgr)
{
    return new struct zmq_pair(mgr);
}

void zmq_pair_factory::destructor(struct block *bk)
{
    delete static_cast<struct zmq_pair *>(bk);
}
