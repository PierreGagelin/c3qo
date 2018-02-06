//
// @brief Implement a AF_UNIX NON-BLOCKING client socket
//          - AF_UNIX      : socket domain and SOCK_STREAM type
//          - NON-BLOCKING : return error code instead of blocking
//
// @note us_asnb stand for unix stream non-block
//

// C++ library headers
#include <cstdio>  // snprintf
#include <cstdlib> // malloc
#include <cstring> // memset

// System library headers
extern "C" {
#include <unistd.h>     // close
#include <sys/types.h>  // getsockopt
#include <sys/un.h>     // sockaddr_un
#include <sys/socket.h> // socket, getsockopt
}

// Project headers
#include "c3qo/block.hpp"
#include "c3qo/manager_fd.hpp"
#include "utils/logger.hpp"
#include "utils/socket.hpp"

#define SOCKET_NAME "/tmp/server_us_nb"

//
// @brief Context of the block
//
struct client_us_nb_ctx
{
    // Configuration
    int bk_id; // Block ID

    // Context
    int fd; // Socket file descriptor

    // Statistics
    size_t rx_pkt_count; // RX: Number of packets read
    size_t rx_pkt_bytes; // RX: Total size read
    size_t tx_pkt_count; // TX: Number of packets sent
    size_t tx_pkt_bytes; // TX: Total size sent
};

//
// @brief Remove a managed file descriptor and close it
//
static void client_us_nb_clean(struct client_us_nb_ctx *ctx)
{
    manager_fd::remove(ctx->fd, true);
    manager_fd::remove(ctx->fd, false);
    close(ctx->fd);
    ctx->fd = -1;
}

//
// @brief Callback function when data is received
//
void client_us_nb_callback(void *vctx, int fd)
{
    struct client_us_nb_ctx *ctx;

    // Verify input
    if (vctx == NULL)
    {
        LOGGER_ERR("Failed to handle file descriptor callback [fd=%d]", fd);
        return;
    }
    ctx = (struct client_us_nb_ctx *)vctx;
    if (fd != ctx->fd)
    {
        LOGGER_ERR("Failed to retrieve file descriptor from context [ctx=%p ; fd=%d]", ctx, fd);
    }

    LOGGER_DEBUG("Handle file descriptor callback [ctx=%p ; fd=%d]", ctx, fd);

    LOGGER_DEBUG("Received data on socket. Not implemented yet [fd=%d]", fd);
}

//
// @brief Check that the socket is connected
//
void client_us_nb_connect_check(void *vctx, int fd)
{
    struct client_us_nb_ctx *ctx;
    socklen_t len;
    int optval;

    // Verify input
    if (vctx == NULL)
    {
        LOGGER_ERR("Failed to check connection status [fd=%d]", fd);
        return;
    }
    ctx = (struct client_us_nb_ctx *)vctx;
    if (fd != ctx->fd)
    {
        LOGGER_ERR("Failed to retrieve file descriptor from context [ctx=%p ; fd=%d]", ctx, fd);
    }

    LOGGER_DEBUG("Check connection status [ctx=%p ; fd=%d]", ctx, ctx->fd);

    // Verify connection status
    len = sizeof(optval);
    if (getsockopt(ctx->fd, SOL_SOCKET, SO_ERROR, (void *)(&optval), &len) != 0)
    {
        LOGGER_ERR("getsockopt failed on socket [fd=%d]", ctx->fd);
        return;
    }

    if (optval != 0)
    {
        LOGGER_ERR("SO_ERROR still not clear on socket [fd=%d]", ctx->fd);
        return;
    }

    LOGGER_DEBUG("Client socket connected to server [ctx=%p ; fd=%d]", ctx, ctx->fd);

    manager_fd::remove(ctx->fd, false);
    manager_fd::add(ctx, ctx->fd, &client_us_nb_callback, true);
}

//
// @brief Connect to a server
//
static int client_us_nb_connect(int fd)
{
    struct sockaddr_un clt_addr;
    int ret;

    memset(&clt_addr, 0, sizeof(clt_addr));
    clt_addr.sun_family = AF_UNIX;
    ret = snprintf(clt_addr.sun_path, sizeof(clt_addr.sun_path), SOCKET_NAME);
    if (ret < 0)
    {
        LOGGER_ERR("Failed snprintf [buf=%p ; size=%lu ; string=%s]", clt_addr.sun_path, sizeof(clt_addr.sun_path), SOCKET_NAME);
        return -1;
    }
    else if (((size_t)ret) > sizeof(clt_addr.sun_path))
    {
        LOGGER_ERR("Failed to write socket name, it's too large [sun_path=%s ; max_size=%lu]", SOCKET_NAME, sizeof(clt_addr.sun_path));
        return -1;
    }

    return c3qo_socket_connect_nb(fd, (struct sockaddr *)&clt_addr, sizeof(clt_addr));
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
        LOGGER_ERR("Failed to initialize block");
        return ctx;
    }

    ctx->bk_id = bk_id;
    ctx->fd = -1;
    ctx->rx_pkt_count = 0;
    ctx->rx_pkt_bytes = 0;
    ctx->tx_pkt_count = 0;
    ctx->tx_pkt_bytes = 0;

    LOGGER_INFO("Initialize block [ctx=%p]", ctx);

    return ctx;
}

//
// @brief Start the block
//
void client_us_nb_start(void *vctx)
{
    struct client_us_nb_ctx *ctx;
    int ret;

    if (vctx == NULL)
    {
        LOGGER_ERR("Failed to start block");
        return;
    }
    ctx = (struct client_us_nb_ctx *)vctx;

    LOGGER_INFO("Start block [ctx=%p]", ctx);

    // Create the client socket
    ctx->fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (ctx->fd == -1)
    {
        LOGGER_ERR("Failed to open client socket [fd=%d]", ctx->fd);
        return;
    }

    // Set the socket to be non-blocking
    c3qo_socket_set_nb(ctx->fd);

    // Connect the socket to the server
    ret = client_us_nb_connect(ctx->fd);
    if (ret == -1)
    {
        LOGGER_ERR("Failed to connect to server [fd=%d]", ctx->fd);
        client_us_nb_clean(ctx);
        return;
    }
    else if (ret == 1)
    {
        LOGGER_DEBUG("Connection sent but not acknowledged, will check later [fd=%d]", ctx->fd);
        manager_fd::add(ctx, ctx->fd, &client_us_nb_connect_check, false);
        return;
    }
    else if (ret == 2)
    {
        LOGGER_ERR("Failed to find someone listening, launch timer for reconnection [fd=%d]", ctx->fd);

        return;
    }
    else
    {
        // Success: register the file descriptor with a callback for data reception
        if (manager_fd::add(ctx, ctx->fd, &client_us_nb_callback, true) == false)
        {
            LOGGER_ERR("Failed to register callback on client socket [fd=%d ; callback=%p]", ctx->fd, &client_us_nb_callback);
            client_us_nb_clean(ctx);
            return;
        }
        else
        {
            LOGGER_DEBUG("Registered callback on client socket [fd=%d ; callback=%p]", ctx->fd, &client_us_nb_callback);
        }
    }
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
}

// Declare the interface for this block
struct bk_if client_us_nb_entry = {
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
