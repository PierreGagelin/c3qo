

// Project headers
#include "c3qo/manager.hpp"

// Managers shall be linked
extern struct manager *m;

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
    if (vctx == NULL)
    {
        LOGGER_ERR("Failed to execute callback: NULL context [socket=%p]", socket);
        return;
    }
    ctx = (struct zmq_pair_ctx *)vctx;
    if (socket != ctx->zmq_sock)
    {
        LOGGER_ERR("Failed to execute ZMQ callback: unknown socket [socket=%p ; expected=%p]", socket, ctx->zmq_sock);
        return;
    }

    msg.topic = NULL;
    msg.data = NULL;

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
    if (msg.topic != NULL)
    {
        free(msg.topic);
    }
    if (msg.data != NULL)
    {
        free(msg.data);
    }
}

//
// @brief Initialize the block context
//
static void *zmq_pair_init(int bk_id)
{
    struct zmq_pair_ctx *ctx;

    // Create a block context
    ctx = new(std::nothrow) struct zmq_pair_ctx;
    if (ctx == nullptr)
    {
        LOGGER_ERR("Failed to initialize block: not enough memory [bk_id=%d]", bk_id);
        return ctx;
    }
    ctx->bk_id = bk_id;

    // Create a ZMQ context
    ctx->zmq_ctx = zmq_ctx_new();
    if (ctx->zmq_ctx == NULL)
    {
        LOGGER_ERR("Failed to initialize block: out of memory for ZMQ context [bk_id=%d]", bk_id);
        free(ctx);
        return NULL;
    }

    // Create a ZMQ socket
    ctx->zmq_sock = zmq_socket(ctx->zmq_ctx, ZMQ_PAIR);
    if (ctx->zmq_sock == NULL)
    {
        LOGGER_ERR("Failed to create ZMQ socket: %s [bk_id=%d ; errno=%d]", strerror(errno), bk_id, errno);
        zmq_ctx_term(ctx->zmq_ctx);
        free(ctx);
        return NULL;
    }

    // Default value for the connection
    ctx->client = false;
    strcpy(ctx->addr, "tcp://127.0.0.1:6666");

    // Initialize statistics
    ctx->rx_pkt_count = 0;
    ctx->tx_pkt_count = 0;

    LOGGER_INFO("Initialize block ZMQ Pair [bk_id=%d]", bk_id);

    return ctx;
}

static void zmq_pair_conf(void *vctx, char *conf)
{
    struct zmq_pair_ctx *ctx;
    char *pos;
    char type[32];
    int ret;

    // Verify input
    if ((vctx == NULL) || (conf == NULL))
    {
        LOGGER_ERR("Failed to configure block: NULL context or conf");
        return;
    }
    ctx = (struct zmq_pair_ctx *)vctx;

    LOGGER_INFO("Configure block [bk_id=%d ; conf=%s]", ctx->bk_id, conf);

    // Retrieve socket type
    {
        pos = strstr(conf, NEEDLE_TYPE);
        if (pos == NULL)
        {
            LOGGER_ERR("Failed to configure block: type of socket is required [bk_id=%d ; conf=%s]", ctx->bk_id, conf);
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
        LOGGER_DEBUG("Configure socket type [bk_id=%d ; type=%s]", ctx->bk_id, type);
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
            LOGGER_ERR("Failed to configure socket type: unknown type [bk_id=%d ; type=%s]", ctx->bk_id, type);
            return;
        }
    }

    // Retrieve socket address
    {
        pos = strstr(conf, NEEDLE_ADDR);
        if (pos == NULL)
        {
            LOGGER_ERR("Failed to configure block: address of socket is required [bk_id=%d ; conf=%s]", ctx->bk_id, conf);
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
static void zmq_pair_start(void *vctx)
{
    struct zmq_pair_ctx *ctx;
    int ret;

    if (vctx == NULL)
    {
        LOGGER_ERR("Failed to start block: NULL context");
        return;
    }
    ctx = (struct zmq_pair_ctx *)vctx;

    LOGGER_INFO("Start block ZMQ Pair [bk_id=%d]", ctx->bk_id);

    // Bind or connect the socket
    if (ctx->client == true)
    {
        ret = zmq_connect(ctx->zmq_sock, ctx->addr);
        if (ret == -1)
        {
            LOGGER_ERR("Failed to connect ZMQ socket: %s [bk_id=%d ; addr=%s ; errno=%d]", strerror(errno), ctx->bk_id, ctx->addr, errno);
            return;
        }
        LOGGER_DEBUG("Connected client socket [bk_id=%d ; addr=%s]", ctx->bk_id, ctx->addr);
    }
    else
    {
        ret = zmq_bind(ctx->zmq_sock, ctx->addr);
        if (ret == -1)
        {
            LOGGER_ERR("Failed to bind ZMQ socket: %s [bk_id=%d ; addr=%s ; errno=%d]", strerror(errno), ctx->bk_id, ctx->addr, errno);
            return;
        }
        LOGGER_DEBUG("Bound server socket [bk_id=%d ; addr=%s]", ctx->bk_id, ctx->addr);
    }

    // Register the subscriber's callback
    m->fd.add(ctx, zmq_pair_callback, -1, ctx->zmq_sock, true);
}

//
// @brief Stop the block
//
static void zmq_pair_stop(void *vctx)
{
    struct zmq_pair_ctx *ctx;

    // Verify input
    if (vctx == NULL)
    {
        LOGGER_ERR("Failed to stop block: NULL context");
        return;
    }
    ctx = (struct zmq_pair_ctx *)vctx;

    // Remove the socket's callback
    m->fd.remove(-1, ctx->zmq_sock, true);

    zmq_close(ctx->zmq_sock);
    zmq_ctx_term(ctx->zmq_ctx);

    LOGGER_INFO("Stop block ZMQ Pair [bk_id=%d]", ctx->bk_id);

    delete ctx;
}

//
// @brief Send data to the exterior
//
static int zmq_pair_tx(void *vctx, void *vdata)
{
    struct zmq_pair_ctx *ctx;
    struct c3qo_zmq_msg *data;
    bool ok;

    if ((vctx == NULL) || (vdata == NULL))
    {
        LOGGER_ERR("Failed to process buffer: NULL context or data");
        return 0;
    }
    ctx = (struct zmq_pair_ctx *)vctx;
    data = (struct c3qo_zmq_msg *)vdata;

    // Send topic and data
    ok = socket_zmq_write(ctx->zmq_sock, data->topic, data->topic_len, ZMQ_DONTWAIT | ZMQ_SNDMORE);
    ok = (ok == true) && socket_zmq_write(ctx->zmq_sock, data->data, data->data_len, ZMQ_DONTWAIT);

    if (ok == true)
    {
        LOGGER_DEBUG("Message sent on ZMQ socket [bk_id=%d ; topic=%s ; payload=%s]", ctx->bk_id, data->topic, data->data);
        ctx->tx_pkt_count++;
    }

    return 0;
}

//
// @brief Exported structure of the block
//
struct bk_if zmq_pair_if = {
    .init = zmq_pair_init,
    .conf = zmq_pair_conf,
    .bind = NULL,
    .start = zmq_pair_start,
    .stop = zmq_pair_stop,

    .get_stats = NULL,

    .rx = NULL,
    .tx = zmq_pair_tx,
    .ctrl = NULL,
};
