

#define LOGGER_TAG "[block.zmq_pair]"

// Project headers
#include "block/hello.hpp"
#include "block/trans_pb.hpp"
#include "block/zmq_pair.hpp"
#include "c3qo/manager.hpp"

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
    struct c3qo_zmq_msg msg;
    bool more;

    if (fd.socket != zmq_sock_.socket)
    {
        LOGGER_ERR("Failed to execute ZMQ callback: unknown socket [socket=%p ; expected=%p]", fd.socket, zmq_sock_.socket);
        return;
    }

    msg.topic = nullptr;
    msg.data = nullptr;

    // Get a topic and payload
    more = socket_zmq_read(zmq_sock_.socket, &msg.topic, &msg.topic_len);
    if (more == false)
    {
        LOGGER_ERR("Failed to receive ZMQ message: expected 2 parts and got 1 [bk_id=%d]", id_);
        goto end;
    }
    more = socket_zmq_read(zmq_sock_.socket, &msg.data, &msg.data_len);
    if (more == true)
    {
        LOGGER_ERR("Failed to receive ZMQ message: expected 2 parts and got more [bk_id=%d]", id_);
        socket_zmq_flush(zmq_sock_.socket);
        goto end;
    }
    rx_pkt_++;

    LOGGER_DEBUG("Message received on ZMQ socket [bk_id=%d ; topic=%s ; payload_size=%zu]", id_, msg.topic, msg.data_len);

    // Action to take upon topic value
    if (strcmp(msg.topic, "CONF.LINE") == 0)
    {
        // Process the configuration line
        mgr_->conf_parse_line(msg.data);
    }
    else if (strcmp(msg.topic, "CONF.PROTO.CMD") == 0)
    {
        mgr_->conf_parse_pb_cmd(reinterpret_cast<uint8_t *>(msg.data), msg.data_len);
    }
    else if (strcmp(msg.topic, "STATS") == 0)
    {
        struct trans_pb_notif notif;

        if (strcmp(msg.data, "HELLO") == 0)
        {
            struct hello_ctx ctx_hello;

            ctx_hello.bk_id = 12;

            notif.type = BLOCK_HELLO;
            notif.context.hello = &ctx_hello;

            process_notif_(1, &notif);
        }
    }
    else
    {
        LOGGER_ERR("Failed to decode ZMQ message: unknown topic [topic=%s]", msg.topic);
        goto end;
    }

end:
    if (msg.topic != nullptr)
    {
        delete[] msg.topic;
    }
    if (msg.data != nullptr)
    {
        delete[] msg.data;
    }
}

void zmq_pair::conf_(char *conf)
{
    char *pos;
    int ret;

    // Verify input
    if (conf == nullptr)
    {
        LOGGER_ERR("Failed to configure block: nullptr conf");
        return;
    }

    LOGGER_INFO("Configure block [bk_id=%d ; conf=%s]", id_, conf);

    // Retrieve socket type
    {
        char type[32];

        pos = strstr(conf, NEEDLE_TYPE);
        if (pos == nullptr)
        {
            LOGGER_ERR("Failed to configure block: type of socket is required [bk_id=%d ; conf=%s]", id_, conf);
            return;
        }

        ret = sscanf(pos, NEEDLE_TYPE "%31s", type);
        if (ret == EOF)
        {
            LOGGER_ERR("Failed to call sscanf: %s [str=%s ; type=%s ; errno=%d]", strerror(errno), pos, type, errno);
            return;
        }
        else if (ret != 1)
        {
            LOGGER_ERR("Failed to call sscanf: wrong number of matched element [expected=1 ; actual=%d ; type=%s]", ret, type);
            return;
        }

        // Configure the socket to be either a client or a server
        LOGGER_INFO("Configure socket type [bk_id=%d ; type=%s]", id_, type);

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
    }

    // Retrieve socket address
    {
        char addr[32];

        pos = strstr(conf, NEEDLE_ADDR);
        if (pos == nullptr)
        {
            LOGGER_ERR("Failed to configure block: address of socket is required [bk_id=%d ; conf=%s]", id_, conf);
            return;
        }

        ret = sscanf(pos, NEEDLE_ADDR "%31s", addr);
        if (ret == EOF)
        {
            LOGGER_ERR("Failed to call sscanf: %s [str=%s ; addr=%s ; errno=%d]", strerror(errno), pos, addr, errno);
            return;
        }
        else if (ret != 1)
        {
            LOGGER_ERR("Failed to call sscanf: wrong number of matched element [expected=1 ; actual=%d ; addr=%s]", ret, addr);
            return;
        }

        addr_ = std::string(addr);

        LOGGER_INFO("Configure socket address [addr=%s]", addr_.c_str());
    }
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
int zmq_pair::tx_(void *vdata)
{
    struct c3qo_zmq_msg *data;
    bool ok;

    if (vdata == nullptr)
    {
        LOGGER_ERR("Failed to process buffer: nullptr context or data");
        return 0;
    }
    data = static_cast<struct c3qo_zmq_msg *>(vdata);

    // Send topic and data
    ok = socket_zmq_write(zmq_sock_.socket, data->topic, data->topic_len, ZMQ_DONTWAIT | ZMQ_SNDMORE);
    ok = (ok == true) && socket_zmq_write(zmq_sock_.socket, data->data, data->data_len, ZMQ_DONTWAIT);

    if (ok == true)
    {
        LOGGER_DEBUG("Message sent on ZMQ socket [bk_id=%d ; topic=%s ; payload=%s]", id_, data->topic, data->data);
        tx_pkt_++;
    }

    return 0;
}

BLOCK_REGISTER(zmq_pair);
