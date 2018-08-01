

// Project headers
#include "c3qo/manager.hpp"

// Managers shall be linked
extern struct manager *m;

bk_zmq_pair::bk_zmq_pair() {}
bk_zmq_pair::~bk_zmq_pair() {}

//
// @brief Callback to handle data available on the socket
//
static void zmq_pair_callback(void *vctx, int fd, void *socket)
{
    struct zmq_pair_ctx *ctx;
    struct c3qo_zmq_msg msg;
    bool more;

    (void)fd;

    // Verify input
    if (vctx == nullptr)
    {
        LOGGER_ERR("Failed to execute callback: nullptr context [socket=%p]", socket);
        return;
    }
    ctx = static_cast<struct zmq_pair_ctx *>(vctx);
    if (socket != ctx->zmq_sock)
    {
        LOGGER_ERR("Failed to execute ZMQ callback: unknown socket [socket=%p ; expected=%p]", socket, ctx->zmq_sock);
        return;
    }

    msg.topic = nullptr;
    msg.data = nullptr;

    // Get a topic and payload
    more = socket_zmq_read(ctx->zmq_sock, &msg.topic, &msg.topic_len);
    if (more == false)
    {
        LOGGER_ERR("Failed to receive ZMQ message: expected 2 parts and got 1 [bk_id=%d]", ctx->bk_id);
        goto end;
    }
    more = socket_zmq_read(ctx->zmq_sock, &msg.data, &msg.data_len);
    if (more == true)
    {
        LOGGER_ERR("Failed to receive ZMQ message: expected 2 parts and got more [bk_id=%d]", ctx->bk_id);
        socket_zmq_flush(ctx->zmq_sock);
        goto end;
    }
    ctx->rx_pkt_count++;

    LOGGER_DEBUG("Message received on ZMQ socket [bk_id=%d ; topic=%s ; payload=%s]", ctx->bk_id, msg.topic, msg.data);

    // Action to take upon topic value
    if (strcmp(msg.topic, "CONF.LINE") == 0)
    {
        // Process the configuration line
        m->bk.conf_parse_line(msg.data);
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

            m->bk.process_notif(ctx->bk_id, 1, &notif);
        }
    }

end:
    if (msg.topic != nullptr)
    {
        free(msg.topic);
    }
    if (msg.data != nullptr)
    {
        free(msg.data);
    }
}

//
// @brief Initialize the block context
//
void bk_zmq_pair::init_()
{
    struct zmq_pair_ctx *ctx;

    // Create a block context
    ctx = new struct zmq_pair_ctx;

    ctx->bk_id = id_;

    // Create a ZMQ context
    ctx->zmq_ctx = zmq_ctx_new();
    assert(ctx->zmq_ctx != nullptr);

    // Create a ZMQ socket
    ctx->zmq_sock = zmq_socket(ctx->zmq_ctx, ZMQ_PAIR);
    assert(ctx->zmq_sock != nullptr);

    // Default value for the connection
    ctx->client = false;
    strcpy(ctx->addr, "tcp://127.0.0.1:6666");

    // Initialize statistics
    ctx->rx_pkt_count = 0;
    ctx->tx_pkt_count = 0;

    LOGGER_INFO("Initialize block ZMQ Pair [bk_id=%d]", id_);

    ctx_ = ctx;
}

void bk_zmq_pair::conf_(char *conf)
{
    struct zmq_pair_ctx *ctx;
    char *pos;
    char type[32];
    int ret;

    // Verify input
    if ((ctx_ == nullptr) || (conf == nullptr))
    {
        LOGGER_ERR("Failed to configure block: nullptr context or conf");
        return;
    }
    ctx = static_cast<struct zmq_pair_ctx *>(ctx_);

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
            LOGGER_ERR("Failed to call sscanf: %s [str=%s ; addr=%s ; errno=%d]", strerror(errno), pos, ctx->addr, errno);
            return;
        }
        else if (ret != 1)
        {
            LOGGER_ERR("Failed to call sscanf: wrong number of matched element [expected=1 ; actual=%d ; addr=%s]", ret, ctx->addr);
            return;
        }

        // Configure the socket to be either a client or a server
        LOGGER_DEBUG("Configure socket type [bk_id=%d ; type=%s]", id_, type);
        if (strcmp(type, "client") == 0)
        {
            ctx->client = true;
        }
        else if (strcmp(type, "server") == 0)
        {
            ctx->client = false;
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

        ret = sscanf(pos, NEEDLE_ADDR ADDR_FORMAT, ctx->addr);
        if (ret == EOF)
        {
            LOGGER_ERR("Failed to call sscanf: %s [str=%s ; addr=%s ; errno=%d]", strerror(errno), pos, ctx->addr, errno);
            return;
        }
        else if (ret != 1)
        {
            LOGGER_ERR("Failed to call sscanf: wrong number of matched element [expected=1 ; actual=%d ; addr=%s]", ret, ctx->addr);
            return;
        }

        LOGGER_INFO("Configure socket address [addr=%s]", ctx->addr);
    }
}

//
// @brief Start the block
//
void bk_zmq_pair::start_()
{
    struct zmq_pair_ctx *ctx;
    int ret;

    if (ctx_ == nullptr)
    {
        LOGGER_ERR("Failed to start block: nullptr context");
        return;
    }
    ctx = static_cast<struct zmq_pair_ctx *>(ctx_);

    LOGGER_INFO("Start block ZMQ Pair [bk_id=%d]", id_);

    // Bind or connect the socket
    if (ctx->client == true)
    {
        ret = zmq_connect(ctx->zmq_sock, ctx->addr);
        if (ret == -1)
        {
            LOGGER_ERR("Failed to connect ZMQ socket: %s [bk_id=%d ; addr=%s ; errno=%d]", strerror(errno), id_, ctx->addr, errno);
            return;
        }
        LOGGER_DEBUG("Connected client socket [bk_id=%d ; addr=%s]", id_, ctx->addr);
    }
    else
    {
        ret = zmq_bind(ctx->zmq_sock, ctx->addr);
        if (ret == -1)
        {
            LOGGER_ERR("Failed to bind ZMQ socket: %s [bk_id=%d ; addr=%s ; errno=%d]", strerror(errno), id_, ctx->addr, errno);
            return;
        }
        LOGGER_DEBUG("Bound server socket [bk_id=%d ; addr=%s]", id_, ctx->addr);
    }

    // Register the subscriber's callback
    m->fd.add(ctx, zmq_pair_callback, -1, ctx->zmq_sock, true);
}

//
// @brief Stop the block
//
void bk_zmq_pair::stop_()
{
    struct zmq_pair_ctx *ctx;

    // Verify input
    if (ctx_ == nullptr)
    {
        LOGGER_ERR("Failed to stop block: nullptr context");
        return;
    }
    ctx = static_cast<struct zmq_pair_ctx *>(ctx_);

    // Remove the socket's callback
    m->fd.remove(-1, ctx->zmq_sock, true);

    zmq_close(ctx->zmq_sock);
    zmq_ctx_term(ctx->zmq_ctx);

    LOGGER_INFO("Stop block ZMQ Pair [bk_id=%d]", id_);

    delete ctx;
}

//
// @brief Send data to the exterior
//
int bk_zmq_pair::tx_(void *vdata)
{
    struct zmq_pair_ctx *ctx;
    struct c3qo_zmq_msg *data;
    bool ok;

    if ((ctx_ == nullptr) || (vdata == nullptr))
    {
        LOGGER_ERR("Failed to process buffer: nullptr context or data");
        return 0;
    }
    ctx = static_cast<struct zmq_pair_ctx *>(ctx_);
    data = static_cast<struct c3qo_zmq_msg *>(vdata);

    // Send topic and data
    ok = socket_zmq_write(ctx->zmq_sock, data->topic, data->topic_len, ZMQ_DONTWAIT | ZMQ_SNDMORE);
    ok = (ok == true) && socket_zmq_write(ctx->zmq_sock, data->data, data->data_len, ZMQ_DONTWAIT);

    if (ok == true)
    {
        LOGGER_DEBUG("Message sent on ZMQ socket [bk_id=%d ; topic=%s ; payload=%s]", id_, data->topic, data->data);
        ctx->tx_pkt_count++;
    }

    return 0;
}
