//
// @brief Implement an AF_UNIX NON-BLOCKING socket
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
#include <unistd.h>     // close, unlink
#include <sys/types.h>  // listen
#include <sys/un.h>     // sockaddr_un
#include <sys/socket.h> // socket, listen
}

// Project headers
#include "block/server_us_nb.hpp"
#include "c3qo/manager_fd.hpp"
#include "utils/logger.hpp"
#include "utils/socket.hpp"

#define SOCKET_READ_SIZE 256
#define SOCKET_NAME "/tmp/server_us_nb"

static inline int server_us_nb_fd_find(struct server_us_nb_ctx *ctx, int fd)
{
    int i;

    for (i = 0; i < SOCKET_FD_MAX; i++)
    {
        if (ctx->fd[i] != fd)
        {
            continue;
        }

        return i;
    }

    return -1;
}

//
// @brief Add a file descriptor to the context
//
// @return -1 on failure, index of input on success
//
static int server_us_nb_fd_add(struct server_us_nb_ctx *ctx, int fd)
{
    int i;

    socket_nb_set(fd);

    if (ctx->fd[ctx->fd_count] != -1)
    {
        // First easy try
        i = ctx->fd_count;
    }
    else
    {
        // Easy try failed, looking for first available
        i = server_us_nb_fd_find(ctx, -1);
    }

    if (i != -1)
    {
        LOGGER_DEBUG("Add file descriptor to block server_us_nb [fd=%d ; fd_count_old=%d ; fd_count_new=%d]", fd, ctx->fd_count, ctx->fd_count + 1);

        ctx->fd[i] = fd;
        ctx->fd_count++;
    }
    else
    {
        LOGGER_ERR("Failed to find room for new file descriptor [fd=%d ; fd_count=%d]", fd, ctx->fd_count);
    }

    return i;
}

//
// @brief Removes a file descriptor
//
// @param i : index where to find the file descriptor
//
static void server_us_nb_remove(struct server_us_nb_ctx *ctx, int i)
{
    // Check bounds and coherency
    if ((i < 0) || (i >= SOCKET_FD_MAX) || (ctx->fd[i] == -1))
    {
        return;
    }

    LOGGER_DEBUG("Remove file descriptor from block server_us_nb [fd=%d ; fd_count_old=%d ; fd_count_new=%d]", ctx->fd[i], ctx->fd_count, ctx->fd_count - 1);

    manager_fd::remove(ctx->fd[i], true);
    close(ctx->fd[i]);
    ctx->fd[i] = -1;
    ctx->fd_count--;
}

//
// @brief Removes a file descriptor
//
// @param fd : file descriptor
//
static inline void server_us_nb_remove_fd(struct server_us_nb_ctx *ctx, int fd)
{
    int i;

    i = server_us_nb_fd_find(ctx, fd);

    if (i != -1)
    {
        server_us_nb_remove(ctx, i);
    }
}

//
// @brief Flush a file descriptor
//
static void server_us_nb_flush_fd(struct server_us_nb_ctx *ctx, int fd)
{
    ssize_t ret;
    char buff[SOCKET_READ_SIZE];

    do
    {
        memset(buff, 0, sizeof(buff));
        ret = socket_nb_read(fd, buff, sizeof(buff));
        if (ret > 0)
        {
            ctx->rx_pkt_count += 1;
            ctx->rx_pkt_bytes += (size_t)ret;
        }
    } while (ret > 0);
}

//
// @brief Callback when a socket is ready for reading
//
// @param fd : file descriptor ready for read
//
static void server_us_nb_handler(void *vctx, int fd)
{
    struct server_us_nb_ctx *ctx;

    if (vctx == NULL)
    {
        LOGGER_ERR("Failed to handle file descriptor callback [fd=%d]", fd);
        return;
    }
    ctx = (struct server_us_nb_ctx *)vctx;

    LOGGER_DEBUG("Handle file descriptor callback [ctx=%p ; fd=%d]", ctx, fd);

    if (fd == ctx->fd[0])
    {
        struct sockaddr_un client;
        socklen_t size;
        int fd_client;

        // New connection has arrived
        size = sizeof(client);
        fd_client = accept(ctx->fd[0], (struct sockaddr *)&client, &size);
        if (fd_client == -1)
        {
            LOGGER_ERR("Failed to accept new client socket [fd_server=%d ; fd_client=%d]", fd, fd_client);
            return;
        }

        // Keep the new file descriptor
        if (server_us_nb_fd_add(ctx, fd_client) == -1)
        {
            LOGGER_ERR("Failed to add new client socket [fd=%d]", fd_client);
            server_us_nb_remove_fd(ctx, fd_client);
            return;
        }

        // Register the fd for event
        if (manager_fd::add(ctx, fd_client, &server_us_nb_handler, true) == false)
        {
            LOGGER_ERR("Failed to register callback on new client socket [fd=%d ; callback=%p]", fd_client, &server_us_nb_handler);
            server_us_nb_remove_fd(ctx, fd_client);
            return;
        }
        else
        {
            LOGGER_DEBUG("Registered callback on new client socket [fd=%d ; callback=%p]", fd_client, &server_us_nb_handler);
        }
    }
    else
    {
        // Data available from the client
        server_us_nb_flush_fd(ctx, fd);
    }
}

//
// @brief Initialize the block
//
void *server_us_nb_init(int bk_id)
{
    struct server_us_nb_ctx *ctx;

    ctx = (struct server_us_nb_ctx *)malloc(sizeof(*ctx));
    if (ctx == NULL)
    {
        LOGGER_ERR("Failed to initialize block: could not reserve memory for the context [bk_id=%d]", bk_id);
        return ctx;
    }

    memset(ctx->fd, -1, sizeof(ctx->fd));
    ctx->bk_id = bk_id;
    ctx->fd_count = 0;
    ctx->rx_pkt_count = 0;
    ctx->rx_pkt_bytes = 0;
    ctx->tx_pkt_count = 0;
    ctx->tx_pkt_bytes = 0;

    // Remove UNIX socket
    unlink(SOCKET_NAME);

    LOGGER_INFO("Initialize block [ctx=%p]", ctx);

    return ctx;
}

//
// @brief Start the block
//
void server_us_nb_start(void *vctx)
{
    struct server_us_nb_ctx *ctx;
    struct sockaddr_un srv_addr;
    int ret;

    if (vctx == NULL)
    {
        LOGGER_ERR("Failed to start block: NULL context");
        return;
    }
    ctx = (struct server_us_nb_ctx *)vctx;

    // Creation of the server socket
    ctx->fd[0] = socket(AF_UNIX, SOCK_STREAM, 0);
    ctx->fd_count++;
    if (ctx->fd[0] == -1)
    {
        LOGGER_ERR("Failed to open server socket [fd=%d]", ctx->fd[0]);
        return;
    }

    // Register the file descriptor for reading
    if (manager_fd::add(ctx, ctx->fd[0], &server_us_nb_handler, true) == false)
    {
        LOGGER_ERR("Failed to register callback on server socket [fd=%d ; callback=%p]", ctx->fd[0], &server_us_nb_handler);
        server_us_nb_remove_fd(ctx, ctx->fd[0]);
        return;
    }

    // Set the socket to be NB
    socket_nb_set(ctx->fd[0]);

    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sun_family = AF_UNIX;
    strcpy(srv_addr.sun_path, SOCKET_NAME);

    // Close an eventual old socket and bind the new one
    unlink(SOCKET_NAME);
    ret = bind(ctx->fd[0], (struct sockaddr *)&srv_addr, sizeof(srv_addr));
    if (ret < 0)
    {
        LOGGER_ERR("Failed to bind server socket [fd=%d]", ctx->fd[0]);
        server_us_nb_remove_fd(ctx, ctx->fd[0]);
        return;
    }

    // Listen on the socket with 5 pending connections maximum
    ret = listen(ctx->fd[0], 5);
    if (ret != 0)
    {
        LOGGER_ERR("Failed to listen on server socket [fd=%d]", ctx->fd[0]);
        server_us_nb_remove_fd(ctx, ctx->fd[0]);
        return;
    }

    LOGGER_DEBUG("Server socket ready to accept clients [fd=%d ; callback=%p]", ctx->fd[0], &server_us_nb_handler);
}

//
// @brief Stop the block
//
void server_us_nb_stop(void *vctx)
{
    struct server_us_nb_ctx *ctx;
    int i;

    if (vctx == NULL)
    {
        LOGGER_ERR("Failed to stop block");
        return;
    }
    ctx = (struct server_us_nb_ctx *)vctx;

    LOGGER_INFO("Stop block [ctx=%p]", ctx);

    // Close every file descriptors
    for (i = 0; i < SOCKET_FD_MAX; i++)
    {
        server_us_nb_remove(ctx, i);
    }

    // Initialize stats
    ctx->rx_pkt_count = 0;
    ctx->rx_pkt_bytes = 0;
    ctx->tx_pkt_count = 0;
    ctx->tx_pkt_bytes = 0;

    // Remove UNIX socket
    unlink(SOCKET_NAME);

    // Free the context structure
    free(ctx);
}

//
// @brief Dump statistics of the block in a string
//
// @param buf : String to dump statistics
// @param len : Size of the string
//
// @return Actual size written
//
size_t server_us_nb_get_stats(void *vctx, char *buf, size_t len)
{
    int ret;
    size_t count;
    struct server_us_nb_ctx *ctx;

    if (vctx == NULL)
    {
        LOGGER_ERR("Failed to get block statistics");
        return 0;
    }
    ctx = (struct server_us_nb_ctx *)vctx;

    LOGGER_DEBUG("Get block statistics [ctx=%p ; buf=%p ; len=%lu]", ctx, buf, len);

    ret = snprintf(buf, len, "%d", ctx->fd_count);
    if (ret < 0)
    {
        LOGGER_ERR("Failed snprintf [ctx=%p ; buf=%p ; len=%lu]", ctx, buf, len);
        return 0;
    }
    else
    {
        count = (size_t)ret;
    }

    if (count > len)
    {
        return len;
    }
    else
    {
        return count;
    }
}

// Declare the interface for this block
struct bk_if server_us_nb_if = {
    .init = server_us_nb_init,
    .conf = NULL,
    .bind = NULL,
    .start = server_us_nb_start,
    .stop = server_us_nb_stop,

    .get_stats = server_us_nb_get_stats,

    .rx = NULL,
    .tx = NULL,
    .ctrl = NULL,
};
