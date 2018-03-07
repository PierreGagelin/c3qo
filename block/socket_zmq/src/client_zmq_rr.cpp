
// System headers
extern "C" {
#include <zmq.h> // zmq_*
}

// C++ library headers
#include <cerrno>  // errno
#include <cstdlib> // size_t, NULL
#include <cstring> // strerror

// Project headers
#include "block/client_zmq_rr.hpp"
#include "c3qo/manager.hpp"
#include "utils/logger.hpp"

// Managers shall be linked
extern struct manager *m;

//
// @brief Callback to handle data available on the socket
//
static void client_zmq_rr_callback(void *vctx, int fd)
{
    struct client_zmq_rr_ctx *ctx;
    char buffer[10];
    int ret;

    if (vctx == NULL)
    {
        LOGGER_ERR("Failed to execute callback: NULL context [fd=%d]", fd);
        return;
    }
    ctx = (struct client_zmq_rr_ctx *)vctx;

    ret = zmq_recv(ctx->zmq_socket, buffer, 10, 0);
    if (ret == -1)
    {
        LOGGER_ERR("Failed to receive data from ZMQ socket: %s [bk_id=%d ; errno=%d]", strerror(errno), ctx->bk_id, errno);
    }
}

//
// @brief Initialize the block context
//
static void *client_zmq_rr_init(int bk_id)
{
    struct client_zmq_rr_ctx *ctx;

    // Create a block context
    ctx = (struct client_zmq_rr_ctx *)malloc(sizeof(*ctx));
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
    ctx->zmq_socket = zmq_socket(ctx->zmq_ctx, ZMQ_REQ);
    if (ctx->zmq_socket == NULL)
    {
        LOGGER_ERR("Failed to initialize block: %s [bk_id=%d ; errno=%d]", strerror(errno), bk_id, errno);
        zmq_ctx_term(ctx->zmq_ctx);
        free(ctx);
        return NULL;
    }

    return ctx;
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

    // Connect to the server
    ret = zmq_connect(ctx->zmq_socket, "tcp://localhost:5555");
    if (ret == -1)
    {
        LOGGER_ERR("Failed to connect ZMQ socket: %s [bk_id=%d ; errno=%d]", strerror(errno), ctx->bk_id, errno);
    }
}

//
// @brief Stop the block
//
static void client_zmq_rr_stop(void *vctx)
{
    struct client_zmq_rr_ctx *ctx;

    if (vctx == NULL)
    {
        LOGGER_ERR("Failed to stop block: NULL context");
        return;
    }
    ctx = (struct client_zmq_rr_ctx *)vctx;

    zmq_ctx_term(ctx->zmq_ctx);
    zmq_close(ctx->zmq_socket);

    free(ctx);
}

static int client_zmq_rr_rx(void *vctx, void *vdata)
{
    (void) vdata;

    // TODO: have zmq_poll to call this
    client_zmq_rr_callback(vctx, 0);

    return 0;
}

//
// @brief Send data to the exterior
//
static int client_zmq_rr_tx(void *vctx, void *vdata)
{
    struct client_zmq_rr_ctx *ctx;
    int ret;

    if (vctx == NULL)
    {
        LOGGER_ERR("Failed to process buffer: NULL context [data=%p]", vdata);
        return 0;
    }
    ctx = (struct client_zmq_rr_ctx *)vctx;

    ret = zmq_send(ctx->zmq_socket, "Hello", 5, 0);
    if (ret == -1)
    {
        LOGGER_ERR("Failed to send data to ZMQ socket: %s [bk_id=%d ; errno=%d]", strerror(errno), ctx->bk_id, errno);
    }

    return 0;
}

//
// @brief Exported structure of the block
//
struct bk_if client_zmq_rr_if = {
    .init = client_zmq_rr_init,
    .conf = NULL,
    .bind = NULL,
    .start = client_zmq_rr_start,
    .stop = client_zmq_rr_stop,

    .get_stats = NULL,

    .rx = client_zmq_rr_rx,
    .tx = client_zmq_rr_tx,
    .ctrl = NULL,
};
