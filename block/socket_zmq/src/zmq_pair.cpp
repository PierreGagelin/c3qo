

// Project headers
#include "c3qo/manager.hpp"

bk_zmq_pair::bk_zmq_pair(struct manager *mgr) : block(mgr), client_(false), addr_("tcp://127.0.0.1:6666"), rx_pkt_(0u), tx_pkt_(0u)
{
    // Create a ZMQ context
    zmq_ctx_ = zmq_ctx_new();
    ASSERT(zmq_ctx_ != nullptr);

    // Create a ZMQ socket
    zmq_sock_ = zmq_socket(zmq_ctx_, ZMQ_PAIR);
    ASSERT(zmq_sock_ != nullptr);
}

bk_zmq_pair::~bk_zmq_pair()
{
    // Close a ZMQ socket
    zmq_close(zmq_sock_);

    // Delete a ZMQ context
    zmq_ctx_term(zmq_ctx_);
}

//
// @brief Callback to handle data available on the socket
//
static void zmq_pair_callback(void *vctx, int fd, void *socket)
{
    struct bk_zmq_pair *bk;
    struct c3qo_zmq_msg msg;
    bool more;

    (void)fd;

    // Verify input
    if (vctx == nullptr)
    {
        LOGGER_ERR("Failed to execute callback: nullptr context [socket=%p]", socket);
        return;
    }
    bk = static_cast<struct bk_zmq_pair *>(vctx);
    if (socket != bk->zmq_sock_)
    {
        LOGGER_ERR("Failed to execute ZMQ callback: unknown socket [socket=%p ; expected=%p]", socket, bk->zmq_sock_);
        return;
    }

    msg.topic = nullptr;
    msg.data = nullptr;

    // Get a topic and payload
    more = socket_zmq_read(bk->zmq_sock_, &msg.topic, &msg.topic_len);
    if (more == false)
    {
        LOGGER_ERR("Failed to receive ZMQ message: expected 2 parts and got 1 [bk_id=%d]", bk->id_);
        goto end;
    }
    more = socket_zmq_read(bk->zmq_sock_, &msg.data, &msg.data_len);
    if (more == true)
    {
        LOGGER_ERR("Failed to receive ZMQ message: expected 2 parts and got more [bk_id=%d]", bk->id_);
        socket_zmq_flush(bk->zmq_sock_);
        goto end;
    }
    bk->rx_pkt_++;

    LOGGER_DEBUG("Message received on ZMQ socket [bk_id=%d ; topic=%s ; payload=%s]", bk->id_, msg.topic, msg.data);

    // Action to take upon topic value
    if (strcmp(msg.topic, "CONF.LINE") == 0)
    {
        // Process the configuration line
        bk->mgr_->conf_parse_line(msg.data);
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

            bk->process_notif_(1, &notif);
        }
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

void bk_zmq_pair::conf_(char *conf)
{
    char *pos;
    char addr[ADDR_SIZE];
    char type[32];
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
        pos = strstr(conf, NEEDLE_TYPE);
        if (pos == nullptr)
        {
            LOGGER_ERR("Failed to configure block: type of socket is required [bk_id=%d ; conf=%s]", id_, conf);
            return;
        }

        ret = sscanf(pos, NEEDLE_TYPE "%32s", type);
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
        pos = strstr(conf, NEEDLE_ADDR);
        if (pos == nullptr)
        {
            LOGGER_ERR("Failed to configure block: address of socket is required [bk_id=%d ; conf=%s]", id_, conf);
            return;
        }

        ret = sscanf(pos, NEEDLE_ADDR ADDR_FORMAT, addr);
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
void bk_zmq_pair::start_()
{
    int ret;

    LOGGER_INFO("Start block ZMQ Pair [bk_id=%d]", id_);

    // Bind or connect the socket
    if (client_ == true)
    {
        ret = zmq_connect(zmq_sock_, addr_.c_str());
        if (ret == -1)
        {
            LOGGER_ERR("Failed to connect ZMQ socket: %s [bk_id=%d ; addr=%s ; errno=%d]", strerror(errno), id_, addr_.c_str(), errno);
            return;
        }
        LOGGER_DEBUG("Connected client socket [bk_id=%d ; addr=%s]", id_, addr_.c_str());
    }
    else
    {
        ret = zmq_bind(zmq_sock_, addr_.c_str());
        if (ret == -1)
        {
            LOGGER_ERR("Failed to bind ZMQ socket: %s [bk_id=%d ; addr=%s ; errno=%d]", strerror(errno), id_, addr_.c_str(), errno);
            return;
        }
        LOGGER_DEBUG("Bound server socket [bk_id=%d ; addr=%s]", id_, addr_.c_str());
    }

    // Register the subscriber's callback
    mgr_->fd_add(this, zmq_pair_callback, -1, zmq_sock_, true);
}

//
// @brief Stop the block
//
void bk_zmq_pair::stop_()
{
    // Remove the socket's callback
    mgr_->fd_remove(-1, zmq_sock_, true);

    LOGGER_INFO("Stop block ZMQ Pair [bk_id=%d]", id_);
}

//
// @brief Send data to the exterior
//
int bk_zmq_pair::tx_(void *vdata)
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
    ok = socket_zmq_write(zmq_sock_, data->topic, data->topic_len, ZMQ_DONTWAIT | ZMQ_SNDMORE);
    ok = (ok == true) && socket_zmq_write(zmq_sock_, data->data, data->data_len, ZMQ_DONTWAIT);

    if (ok == true)
    {
        LOGGER_DEBUG("Message sent on ZMQ socket [bk_id=%d ; topic=%s ; payload=%s]", id_, data->topic, data->data);
        tx_pkt_++;
    }

    return 0;
}
