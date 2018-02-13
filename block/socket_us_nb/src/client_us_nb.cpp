//
// @brief Implement a AF_UNIX NON-BLOCKING client socket
//          - AF_UNIX      : socket domain and SOCK_STREAM type
//          - NON-BLOCKING : return error code instead of blocking
//
// @note us_asnb stand for unix stream non-block
//

// C++ library headers
#include <cstdio>  // snprintf
#include <cstring> // memset

// System library headers
extern "C" {
#include <unistd.h>     // close
#include <sys/types.h>  // getsockopt
#include <sys/un.h>     // sockaddr_un
#include <sys/socket.h> // socket, getsockopt
}

// Project headers
#include "block/client_us_nb.hpp"
#include "c3qo/manager_bk.hpp"
#include "c3qo/manager_fd.hpp"
#include "c3qo/manager_tm.hpp"
#include "utils/logger.hpp"
#include "utils/socket.hpp"

// One function uses the other: need definition
static void client_us_nb_connect(struct client_us_nb_ctx *ctx);
static void client_us_nb_connect_retry(void *vctx);

// Managers shall be linked
extern class manager_bk m_bk;
extern class manager_tm m_tm;
extern class manager_fd m_fd;

#define SOCKET_NAME "/tmp/server_us_nb"

//
// @brief Remove the managed file descriptor and close it
//
static void client_us_nb_clean(struct client_us_nb_ctx *ctx)
{
    LOGGER_INFO("Remove socket from block context [bk_id=%d ; fd=%d]", ctx->bk_id, ctx->fd);

    m_fd.remove(ctx->fd, true);
    m_fd.remove(ctx->fd, false);
    close(ctx->fd);
    ctx->fd = -1;
}

//
// @brief Callback function when data is received
//
static void client_us_nb_callback(void *vctx, int fd)
{
    struct client_us_nb_ctx *ctx;
    ssize_t ret;

    // Verify input
    if (vctx == NULL)
    {
        LOGGER_ERR("Failed to handle file descriptor callback: NULL context [fd=%d]", fd);
        return;
    }
    ctx = (struct client_us_nb_ctx *)vctx;
    if (fd != ctx->fd)
    {
        LOGGER_ERR("Failed to handle file descriptor callback: unknown file descriptor [bk_id=%d ; fd_exp=%d ; fd_recv=%d]", ctx->bk_id, ctx->fd, fd);
    }

    LOGGER_DEBUG("Handle file descriptor callback [bk_id=%d ; fd=%d]", ctx->bk_id, fd);

    // Flush the file descriptor
    do
    {
        ret = socket_nb_read(ctx->fd, ctx->buf, sizeof(ctx->buf));
        if (ret > 0)
        {
            LOGGER_DEBUG("Received data on socket [bk_id=%d ; fd=%d ; buf=%p ; size=%zd]", ctx->bk_id, ctx->fd, ctx->buf, ret);
            // For the moment this is OK because the data flow is synchronous
            // Need to fix it if asynchronous data flow arrives
            m_bk.process_rx(ctx->bind, ctx->buf);
        }
    } while (ret > 0);
}

//
// @brief Check if the socket is connected
//
static void client_us_nb_connect_check(void *vctx, int fd)
{
    struct client_us_nb_ctx *ctx;

    // Verify input
    if (vctx == NULL)
    {
        LOGGER_ERR("Failed to check socket connection status: NULL context [fd=%d]", fd);
        return;
    }
    ctx = (struct client_us_nb_ctx *)vctx;
    if (fd != ctx->fd)
    {
        LOGGER_ERR("Failed to check socket connection status: unknown file descriptor [bk_id=%d ; fd_exp=%d ; fd_recv=%d]", ctx->bk_id, ctx->fd, fd);
    }

    if (socket_nb_connect_check(ctx->fd) == true)
    {
        // Socket is connected, no need to look for write occasion anymore
        ctx->connected = true;
        m_fd.remove(ctx->fd, false);
        m_fd.add(ctx, ctx->fd, &client_us_nb_callback, true);
    }
}

//
// @brief Try to connect the socket again. It is more portable to create a new one
//
static void client_us_nb_connect_retry(void *vctx)
{
    struct client_us_nb_ctx *ctx;

    // Verify input
    if (vctx == NULL)
    {
        LOGGER_ERR("Failed to retry connection on socket: NULL context");
        return;
    }
    ctx = (struct client_us_nb_ctx *)vctx;

    if (close(ctx->fd) == -1)
    {
        LOGGER_WARNING("Could not close file descriptor properly: I/O might be pending and lost [bk_id=%d ; fd=%d]", ctx->bk_id, ctx->fd);
    }
    // TODO: put the socket options in the configuration
    ctx->fd = socket_nb(AF_UNIX, SOCK_STREAM, 0);
    if (ctx->fd == -1)
    {
        LOGGER_ERR("Failed to retry connection on socket: could not create non-blocking socket [bk_id=%d ; fd=%d]", ctx->bk_id, ctx->fd);
    }

    client_us_nb_connect(ctx);
}

//
// @brief Connect to a server
//
static void client_us_nb_connect(struct client_us_nb_ctx *ctx)
{
    struct sockaddr_un clt_addr;
    int ret;

    // Prepare socket structure
    memset(&clt_addr, 0, sizeof(clt_addr));
    clt_addr.sun_family = AF_UNIX;
    ret = snprintf(clt_addr.sun_path, sizeof(clt_addr.sun_path), SOCKET_NAME);
    if (ret < 0)
    {
        LOGGER_ERR("Failed to connect socket: snprintf error [buf=%p ; size=%lu ; string=%s]", clt_addr.sun_path, sizeof(clt_addr.sun_path), SOCKET_NAME);
        return;
    }
    else if (((size_t)ret) > sizeof(clt_addr.sun_path))
    {
        LOGGER_ERR("Failed to connect socket: socket name too long [sun_path=%s ; max_size=%lu]", SOCKET_NAME, sizeof(clt_addr.sun_path));
        return;
    }

    // Connect the socket
    ret = socket_nb_connect(ctx->fd, (struct sockaddr *)&clt_addr, sizeof(clt_addr));
    switch (ret)
    {
    case 1:
        // Connection in progress, register file descriptor for writing to check the connection when it's ready
        m_fd.add(ctx, ctx->fd, &client_us_nb_connect_check, false);
        break;

    case -1:
    case 2:
        struct timer tm;

        LOGGER_DEBUG("Prepare a timer for socket connection retry [bk_id=%d ; fd=%d]", ctx->bk_id, ctx->fd)

        // Prepare a 100ms timer for connection retry
        tm.tid = ctx->fd;
        tm.callback = &client_us_nb_connect_retry;
        tm.arg = ctx;
        tm.time.tv_sec = 0;
        tm.time.tv_nsec = 100 * 1000 * 1000;
        m_tm.add(tm);
        break;

    case 0:
        // Success: register the file descriptor with a callback for data reception
        if (m_fd.add(ctx, ctx->fd, &client_us_nb_callback, true) == false)
        {
            LOGGER_ERR("Failed to register callback on client socket [fd=%d ; callback=%p]", ctx->fd, &client_us_nb_callback);
            client_us_nb_clean(ctx);
        }
        else
        {
            LOGGER_DEBUG("Registered callback on client socket [fd=%d ; callback=%p]", ctx->fd, &client_us_nb_callback);
            ctx->connected = true;
        }
        break;
    }
}

//
// @brief Initialize the block
//
void *client_us_nb_init(int bk_id)
{
    struct client_us_nb_ctx *ctx;

    // Reserve memory for the context
    ctx = (struct client_us_nb_ctx *)malloc(sizeof(*ctx));
    if (ctx == NULL)
    {
        LOGGER_ERR("Failed to initialize block: could not reserve memory for the context [bk_id=%d]", bk_id);
        return ctx;
    }

    ctx->bk_id = bk_id;

    ctx->fd = -1;
    ctx->connected = false;
    // ctx->buf: no need to initialize

    ctx->rx_pkt_count = 0;
    ctx->rx_pkt_bytes = 0;
    ctx->tx_pkt_count = 0;
    ctx->tx_pkt_bytes = 0;

    return ctx;
}

//
// @brief Start the block
//
void client_us_nb_start(void *vctx)
{
    struct client_us_nb_ctx *ctx;

    if (vctx == NULL)
    {
        LOGGER_ERR("Failed to start block: NULL context");
        return;
    }
    ctx = (struct client_us_nb_ctx *)vctx;

    // Create the client socket
    // TODO: put the socket options in the configuration
    ctx->fd = socket_nb(AF_UNIX, SOCK_STREAM, 0);
    if (ctx->fd == -1)
    {
        LOGGER_ERR("Failed to start block: could not create non-blocking socket [fd=%d]", ctx->fd);
        return;
    }

    // Connect the socket to the server
    client_us_nb_connect(ctx);
}

//
// @brief Stop the block
//
void client_us_nb_stop(void *vctx)
{
    struct client_us_nb_ctx *ctx;

    if (vctx == NULL)
    {
        LOGGER_ERR("Failed to stop block");
        return;
    }
    ctx = (struct client_us_nb_ctx *)vctx;

    LOGGER_INFO("Stop block [ctx=%p]", ctx);

    if (ctx->fd == -1)
    {
        return;
    }

    client_us_nb_clean(ctx);

    // Free the context structure
    free(ctx);
}

// Declare the interface for this block
struct bk_if client_us_nb_if = {
    .init = client_us_nb_init,
    .conf = NULL,
    .bind = NULL,
    .start = client_us_nb_start,
    .stop = client_us_nb_stop,

    .get_stats = NULL,

    .rx = NULL,
    .tx = NULL,
    .ctrl = NULL,
};
