
// System headers
extern "C" {
#include <zmq.h> // zmq_*
}

// C++ library headers
#include <cerrno>  // errno
#include <cstdlib> // size_t, NULL
#include <cstring> // strerror

// Project headers
#include "block/server_zmq_rr.hpp"
#include "c3qo/manager.hpp"
#include "utils/logger.hpp"

// Managers shall be linked
extern struct manager *m;

//
// @brief Callback to handle data available on the socket
//
static void server_zmq_rr_callback(void *vctx, int fd, void *socket)
{
    struct server_zmq_rr_ctx *ctx;
    char buffer[10];
    int ret;

    (void)fd;

    // Verify input
    if (vctx == NULL)
    {
        LOGGER_ERR("Failed to execute callback: NULL context [socket=%p]", socket);
        return;
    }
    ctx = (struct server_zmq_rr_ctx *)vctx;
    if (socket != ctx->zmq_socket)
    {
        LOGGER_ERR("Failed to execute ZMQ callback: unknown socket [socket=%p ; expected=%p]", socket, ctx->zmq_socket);
        return;
    }

    ret = zmq_recv(ctx->zmq_socket, buffer, 10, ZMQ_DONTWAIT);
    if (ret == -1)
    {
        LOGGER_ERR("Failed to receive data from ZMQ socket: %s [bk_id=%d ; errno=%d]", strerror(errno), ctx->bk_id, errno);
    }

    LOGGER_DEBUG("Data available on ZMQ socket [socket=%p]", ctx->zmq_socket);

    ctx->rx_pkt_count++;
}

//
// @brief Initialize the block context
//
static void *server_zmq_rr_init(int bk_id)
{
    struct server_zmq_rr_ctx *ctx;

    // Create a block context
    ctx = (struct server_zmq_rr_ctx *)malloc(sizeof(*ctx));
    if (ctx == NULL)
    {
        LOGGER_ERR("Failed to initialize block: out of memory [bk_id=%d]", bk_id);
        return NULL;
    }
    ctx->bk_id = bk_id;

    // Create a ZMQ context
    ctx->zmq_ctx = zmq_ctx_new();
    if (ctx->zmq_ctx == NULL)
    {
        LOGGER_ERR("Failed to initialize block: out of memory [bk_id=%d]", bk_id);
        free(ctx);
        return NULL;
    }

    // Create a ZMQ socket
    ctx->zmq_socket = zmq_socket(ctx->zmq_ctx, ZMQ_REP);
    if (ctx->zmq_socket == NULL)
    {
        LOGGER_ERR("Failed to initialize block: %s [bk_id=%d ; errno=%d]", strerror(errno), bk_id, errno);
        zmq_ctx_term(ctx->zmq_ctx);
        free(ctx);
        return NULL;
    }

    // Bind and listen to an endpoint
    zmq_bind(ctx->zmq_socket, "tcp://*:5555");

    // Initialize statistics
    ctx->rx_pkt_count = 0;
    ctx->tx_pkt_count = 0;

    LOGGER_INFO("Initialize block server ZMQ Request-Reply [bk_id=%d]", bk_id);

    return ctx;
}

//
// @brief Start the block
//
static void server_zmq_rr_start(void *vctx)
{
    struct server_zmq_rr_ctx *ctx;

    // Verify input
    if (vctx == NULL)
    {
        LOGGER_ERR("Failed to start block: NULL context");
        return;
    }
    ctx = (struct server_zmq_rr_ctx *)vctx;

    // Register the socket's callback
    m->fd.add(ctx, server_zmq_rr_callback, -1, ctx->zmq_socket, true);

    LOGGER_INFO("Start block server ZMQ Request-Reply [bk_id=%d]", ctx->bk_id);
}

//
// @brief Stop the block
//
static void server_zmq_rr_stop(void *vctx)
{
    struct server_zmq_rr_ctx *ctx;

    // Verify input
    if (vctx == NULL)
    {
        LOGGER_ERR("Failed to stop block: NULL context");
        return;
    }
    ctx = (struct server_zmq_rr_ctx *)vctx;

    // Remove the socket's callback
    m->fd.remove(-1, ctx->zmq_socket, true);

    zmq_close(ctx->zmq_socket);
    zmq_ctx_term(ctx->zmq_ctx);

    free(ctx);

    LOGGER_INFO("Stop block server ZMQ Request-Reply [bk_id=%d]", ctx->bk_id);
}

//
// @brief Send data to the exterior
//
static int server_zmq_rr_tx(void *vctx, void *vdata)
{
    struct server_zmq_rr_ctx *ctx;
    int ret;

    if (vctx == NULL)
    {
        LOGGER_ERR("Failed to process buffer: NULL context [data=%p]", vdata);
        return 0;
    }
    ctx = (struct server_zmq_rr_ctx *)vctx;

    ret = zmq_send(ctx->zmq_socket, "World", 5, ZMQ_DONTWAIT);
    if (ret == -1)
    {
        LOGGER_ERR("Failed to send data to ZMQ socket: %s [bk_id=%d ; errno=%d]", strerror(errno), ctx->bk_id, errno);
    }

    ctx->tx_pkt_count++;

    return 0;
}

//
// @brief Exported structure of the block
//
struct bk_if server_zmq_rr_if = {
    .init = server_zmq_rr_init,
    .conf = NULL,
    .bind = NULL,
    .start = server_zmq_rr_start,
    .stop = server_zmq_rr_stop,

    .get_stats = NULL,

    .rx = NULL,
    .tx = server_zmq_rr_tx,
    .ctrl = NULL,
};
