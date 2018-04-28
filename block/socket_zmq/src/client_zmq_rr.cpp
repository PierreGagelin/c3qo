
// System library headers
extern "C" {
#include <zmq.h> // zmq_*
}

// C++ library headers
#include <cerrno>  // errno
#include <cstdio>  // sscanf
#include <cstdlib> // size_t, NULL
#include <cstring> // strerror

// Project headers
#include "block/client_zmq_rr.hpp"
#include "c3qo/manager.hpp"
#include "utils/logger.hpp"
#include "utils/socket.hpp"

// Managers shall be linked
extern struct manager *m;

//
// @brief Callback to handle data available on the socket
//
static void client_zmq_rr_callback(void *vctx, int fd, void *socket)
{
    struct client_zmq_rr_ctx *ctx;
    struct c3qo_zmq_msg msg;
    bool ret;

    (void)fd;

    // Verify input
    if (vctx == NULL)
    {
        LOGGER_ERR("Failed to execute callback: NULL context [socket=%p]", socket);
        return;
    }
    ctx = (struct client_zmq_rr_ctx *)vctx;
    if (socket != ctx->zmq_socket_sub)
    {
        LOGGER_ERR("Failed to execute ZMQ callback: unknown socket [socket=%p ; expected=%p]", socket, ctx->zmq_socket_sub);
        return;
    }

    ret = socket_nb_zmq_read(ctx->zmq_socket_sub, &msg.topic, &msg.topic_len);
    if (ret == false)
    {
        LOGGER_ERR("Failed to receive data from ZMQ socket: %s [bk_id=%d ; errno=%d]", strerror(errno), ctx->bk_id, errno);
    }
    ret = socket_nb_zmq_read(ctx->zmq_socket_sub, &msg.data, &msg.data_len);
    if (ret == true)
    {
        LOGGER_ERR("Failed to receive data from ZMQ socket: %s [bk_id=%d ; errno=%d]", strerror(errno), ctx->bk_id, errno);
    }

    LOGGER_DEBUG("Data available on ZMQ socket [socket=%p]", ctx->zmq_socket_sub);

    ctx->rx_pkt_count++;
}

//
// @brief Initialize the block context
//
static void *client_zmq_rr_init(int bk_id)
{
    struct client_zmq_rr_ctx *ctx;
    int ret;

    // Create a block context
    ctx = (struct client_zmq_rr_ctx *)malloc(sizeof(*ctx));
    if (ctx == NULL)
    {
        LOGGER_ERR("Failed to initialize block: %s [bk_id=%d ; errno=%d]", strerror(errno), bk_id, errno);
        return NULL;
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

    // Create a publisher and a subscriber
    ctx->zmq_socket_pub = zmq_socket(ctx->zmq_ctx, ZMQ_PUB);
    if (ctx->zmq_socket_pub == NULL)
    {
        LOGGER_ERR("Failed to create ZMQ socket: %s [bk_id=%d ; errno=%d]", strerror(errno), bk_id, errno);
        zmq_ctx_term(ctx->zmq_ctx);
        free(ctx);
        return NULL;
    }
    ctx->zmq_socket_sub = zmq_socket(ctx->zmq_ctx, ZMQ_SUB);
    if (ctx->zmq_socket_sub == NULL)
    {
        LOGGER_ERR("Failed to create ZMQ socket: %s [bk_id=%d ; errno=%d]", strerror(errno), bk_id, errno);
        zmq_close(ctx->zmq_socket_pub);
        zmq_ctx_term(ctx->zmq_ctx);
        free(ctx);
        return NULL;
    }

    // No filter for the subscriber
    ret = zmq_setsockopt(ctx->zmq_socket_sub, ZMQ_SUBSCRIBE, "", 0);
    if (ret == -1)
    {
        LOGGER_ERR("Failed to set ZMQ socket option: %s [bk_id=%d ; errno=%d]", strerror(errno), ctx->bk_id, errno);
        zmq_close(ctx->zmq_socket_pub);
        zmq_close(ctx->zmq_socket_sub);
        zmq_ctx_term(ctx->zmq_ctx);
        free(ctx);
        return NULL;
    }

    // Initialize statistics
    ctx->rx_pkt_count = 0;
    ctx->tx_pkt_count = 0;

    LOGGER_INFO("Initialize block ZMQ Publish/Subscribe [bk_id=%d]", bk_id);

    return ctx;
}

static void client_zmq_conf(void *vctx, char *conf)
{
    struct client_zmq_rr_ctx *ctx;
    char *pos;

    // Verify input
    if ((vctx == NULL) || (conf == NULL))
    {
        LOGGER_ERR("Failed to configure block: NULL context or conf");
        return;
    }
    ctx = (struct client_zmq_rr_ctx *)vctx;

    LOGGER_INFO("Configure block [bk_id=%d ; conf=%s]", ctx->bk_id, conf);

    // Retrieve publisher and subscriber addresses
    pos = strstr(conf, NEEDLE_PUB);
    if (pos != NULL)
    {
        int ret;

        ret = sscanf(pos, NEEDLE_PUB ADDR_FORMAT, ctx->pub_addr);
        if (ret == EOF)
        {
            LOGGER_ERR("Failed to call sscanf: %s [str=%s ; pub_addr=%s ; errno=%d]", strerror(errno), pos, ctx->pub_addr, errno);
        }
        else if (ret != 1)
        {
            LOGGER_ERR("Failed to call sscanf: wrong number of matched element [expected=1 ; actual=%d ; pub_addr=%s]", ret, ctx->pub_addr);
        }
        else
        {
            LOGGER_INFO("Configure publisher address [pub_addr=%s]", ctx->pub_addr);

            // Bind the publisher
            ret = zmq_bind(ctx->zmq_socket_pub, ctx->pub_addr);
            if (ret == -1)
            {
                LOGGER_ERR("Failed to bind ZMQ publisher socket: %s [bk_id=%d ; errno=%d]", strerror(errno), ctx->bk_id, errno);
            }
            LOGGER_DEBUG("Bound ZMQ publisher socket [pub_addr=%s]", ctx->pub_addr);
        }
    }
    pos = strstr(conf, NEEDLE_SUB);
    if (pos != NULL)
    {
        int ret;

        ret = sscanf(pos, NEEDLE_SUB ADDR_FORMAT, ctx->sub_addr);
        if (ret == EOF)
        {
            LOGGER_ERR("Failed to call sscanf: %s [str=%s ; sub_addr=%s ; errno=%d]", strerror(errno), pos, ctx->sub_addr, errno);
        }
        else if (ret != 1)
        {
            LOGGER_ERR("Failed to call sscanf: wrong number of matched element [expected=1 ; actual=%d ; sub_addr=%s]", ret, ctx->sub_addr);
        }
        else
        {
            LOGGER_INFO("Configure subscriber address [sub_addr=%s]", ctx->sub_addr);
        }
    }
}

//
// @brief Start the block
//
static void client_zmq_rr_start(void *vctx)
{
    struct client_zmq_rr_ctx *ctx;
    int ret;

    if (vctx == NULL)
    {
        LOGGER_ERR("Failed to start block: NULL context");
        return;
    }
    ctx = (struct client_zmq_rr_ctx *)vctx;

    // Connect the subscriber
    ret = zmq_connect(ctx->zmq_socket_sub, ctx->sub_addr);
    if (ret == -1)
    {
        LOGGER_ERR("Failed to connect ZMQ socket: %s [bk_id=%d ; errno=%d]", strerror(errno), ctx->bk_id, errno);
    }

    // Register the subscriber's callback
    m->fd.add(ctx, client_zmq_rr_callback, -1, ctx->zmq_socket_sub, true);

    LOGGER_INFO("Start block ZMQ Publish/Subscribe [bk_id=%d]", ctx->bk_id);
}

//
// @brief Stop the block
//
static void client_zmq_rr_stop(void *vctx)
{
    struct client_zmq_rr_ctx *ctx;

    // Verify input
    if (vctx == NULL)
    {
        LOGGER_ERR("Failed to stop block: NULL context");
        return;
    }
    ctx = (struct client_zmq_rr_ctx *)vctx;

    // Remove the socket's callback
    m->fd.remove(-1, ctx->zmq_socket_sub, true);

    zmq_close(ctx->zmq_socket_sub);
    zmq_close(ctx->zmq_socket_pub);
    zmq_ctx_term(ctx->zmq_ctx);

    free(ctx);

    LOGGER_INFO("Stop block ZMQ Publish/Subscribe [bk_id=%d]", ctx->bk_id);
}

//
// @brief Send data to the exterior
//
static int client_zmq_rr_tx(void *vctx, void *vdata)
{
    struct client_zmq_rr_ctx *ctx;
    zmq_msg_t topic;
    zmq_msg_t msg;
    int ret;

    if (vctx == NULL)
    {
        LOGGER_ERR("Failed to process buffer: NULL context [data=%p]", vdata);
        return 0;
    }
    ctx = (struct client_zmq_rr_ctx *)vctx;

    zmq_msg_init_size(&topic, 5);
    memcpy(zmq_msg_data(&topic), "Hello", 5);
    ret = zmq_msg_send(&topic, ctx->zmq_socket_pub, ZMQ_DONTWAIT | ZMQ_SNDMORE);
    if (ret == -1)
    {
        LOGGER_ERR("Failed to send data to ZMQ socket: %s [bk_id=%d ; errno=%d]", strerror(errno), ctx->bk_id, errno);
        zmq_msg_close(&topic);
        return 0;
    }

    zmq_msg_init_size(&msg, 5);
    memcpy(zmq_msg_data(&msg), "World", 5);
    ret = zmq_msg_send(&msg, ctx->zmq_socket_pub, ZMQ_DONTWAIT);
    if (ret == -1)
    {
        LOGGER_ERR("Failed to send data to ZMQ socket: %s [bk_id=%d ; errno=%d]", strerror(errno), ctx->bk_id, errno);
        zmq_msg_close(&msg);
        return 0;
    }

    ctx->tx_pkt_count++;

    return 0;
}

//
// @brief Exported structure of the block
//
struct bk_if client_zmq_rr_if = {
    .init = client_zmq_rr_init,
    .conf = client_zmq_conf,
    .bind = NULL,
    .start = client_zmq_rr_start,
    .stop = client_zmq_rr_stop,

    .get_stats = NULL,

    .rx = NULL,
    .tx = client_zmq_rr_tx,
    .ctrl = NULL,
};
