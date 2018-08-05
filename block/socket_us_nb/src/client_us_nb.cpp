//
// @brief Implement a AF_UNIX NON-BLOCKING client socket
//          - AF_UNIX      : socket domain and SOCK_STREAM type
//          - NON-BLOCKING : return error code instead of blocking
//
// @note us_nb stands for unix stream non-block
//

// Project headers
#include "c3qo/manager.hpp"

bk_client_us_nb::bk_client_us_nb(struct manager *mgr) : block(mgr) {}

#define SOCKET_NAME "/tmp/server_us_nb"
#define SOCKET_READ_SIZE 256

//
// @brief Remove the managed file descriptor and close it
//
void bk_client_us_nb::clean_()
{
    struct client_us_nb_ctx *ctx;

    ctx = static_cast<struct client_us_nb_ctx *>(ctx_);

    LOGGER_INFO("Remove socket from block context [bk_id=%d ; fd=%d]", ctx->bk_id, ctx->fd);

    mgr_->fd_remove(ctx->fd, nullptr, true);
    mgr_->fd_remove(ctx->fd, nullptr, false);
    close(ctx->fd);
    ctx->fd = -1;
}

//
// @brief Callback function when data is received
//
static void client_us_nb_callback(void *vctx, int fd, void *socket)
{
    struct block *bk;
    struct client_us_nb_ctx *ctx;
    char buf[SOCKET_READ_SIZE];
    ssize_t ret;

    (void) socket;

    // Verify input
    if (vctx == nullptr)
    {
        LOGGER_ERR("Failed to handle file descriptor callback: nullptr context [fd=%d]", fd);
        return;
    }
    bk = static_cast<struct block *>(vctx);
    ctx = static_cast<struct client_us_nb_ctx *>(bk->ctx_);
    if (fd != ctx->fd)
    {
        LOGGER_ERR("Failed to handle file descriptor callback: unknown file descriptor [bk_id=%d ; fd_exp=%d ; fd_recv=%d]", ctx->bk_id, ctx->fd, fd);
    }

    LOGGER_DEBUG("Handle file descriptor callback [bk_id=%d ; fd=%d]", ctx->bk_id, fd);

    // Flush the file descriptor
    do
    {
        ret = socket_nb_read(ctx->fd, buf, sizeof(buf));
        if (ret > 0)
        {
            LOGGER_DEBUG("Received data on socket [bk_id=%d ; fd=%d ; buf=%p ; size=%zd]", ctx->bk_id, ctx->fd, buf, ret);

            ctx->rx_pkt_count++;
            ctx->rx_pkt_bytes += static_cast<size_t>(ret);

            // For the moment this is OK because
            //   - the data flow is synchronous
            //   - only one block is bound
            bk->process_rx_(1, buf);
        }
    } while (ret > 0);
}

//
// @brief Check if the socket is connected
//
static void client_us_nb_connect_check(void *vctx, int fd, void *socket)
{
    struct block *bk;
    struct client_us_nb_ctx *ctx;

    (void) socket;

    // Verify input
    if (vctx == nullptr)
    {
        LOGGER_ERR("Failed to check socket connection status: nullptr context [fd=%d]", fd);
        return;
    }
    bk = static_cast<struct block *>(vctx);
    ctx = static_cast<struct client_us_nb_ctx *>(bk->ctx_);
    if (fd != ctx->fd)
    {
        LOGGER_ERR("Failed to check socket connection status: unknown file descriptor [bk_id=%d ; fd_exp=%d ; fd_recv=%d]", ctx->bk_id, ctx->fd, fd);
    }

    if (socket_nb_connect_check(ctx->fd) == true)
    {
        // Socket is connected, no need to look for write occasion anymore
        ctx->connected = true;
        bk->mgr_->fd_remove(ctx->fd, nullptr, false);
        bk->mgr_->fd_add(bk, &client_us_nb_callback, ctx->fd, nullptr, true);
    }
}

//
// @brief Try to connect the socket again. It is more portable to create a new one
//
static void client_us_nb_connect_retry(void *vctx)
{
    struct bk_client_us_nb *bk;
    struct client_us_nb_ctx *ctx;

    // Verify input
    if (vctx == nullptr)
    {
        LOGGER_ERR("Failed to retry connection on socket: nullptr context");
        return;
    }
    bk = static_cast<struct bk_client_us_nb *>(vctx);
    ctx = static_cast<struct client_us_nb_ctx *>(bk->ctx_);

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

    bk->connect_();
}

//
// @brief Connect to a server
//
void bk_client_us_nb::connect_()
{
    struct client_us_nb_ctx *ctx;
    struct sockaddr_un clt_addr;
    int ret;

    ctx = static_cast<struct client_us_nb_ctx *>(ctx_);

    // Prepare socket structure
    memset(&clt_addr, 0, sizeof(clt_addr));
    clt_addr.sun_family = AF_UNIX;
    ret = snprintf(clt_addr.sun_path, sizeof(clt_addr.sun_path), SOCKET_NAME);
    if (ret < 0)
    {
        LOGGER_ERR("Failed to connect socket: snprintf error [buf=%p ; size=%lu ; string=%s]", clt_addr.sun_path, sizeof(clt_addr.sun_path), SOCKET_NAME);
        return;
    }
    else if ((static_cast<size_t>(ret)) > sizeof(clt_addr.sun_path))
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
        mgr_->fd_add(this, &client_us_nb_connect_check, ctx->fd, nullptr, false);
        break;

    case -1:
    case 2:
        struct timer tm;

        LOGGER_DEBUG("Prepare a timer for socket connection retry [bk_id=%d ; fd=%d]", ctx->bk_id, ctx->fd);

        // Prepare a 100ms timer for connection retry
        tm.tid = ctx->fd;
        tm.callback = &client_us_nb_connect_retry;
        tm.arg = this;
        tm.time.tv_sec = 0;
        tm.time.tv_nsec = 100 * 1000 * 1000;
        mgr_->timer_add(tm);
        break;

    case 0:
        // Success: register the file descriptor with a callback for data reception
        if (mgr_->fd_add(this, &client_us_nb_callback, ctx->fd, nullptr, true) == false)
        {
            LOGGER_ERR("Failed to register callback on client socket [fd=%d ; callback=%p]", ctx->fd, &client_us_nb_callback);
            clean_();
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
void bk_client_us_nb::init_()
{
    struct client_us_nb_ctx *ctx;

    // Reserve memory for the context
    ctx = new struct client_us_nb_ctx;

    ctx->bk_id = id_;

    ctx->fd = -1;
    ctx->connected = false;

    ctx->rx_pkt_count = 0;
    ctx->rx_pkt_bytes = 0;
    ctx->tx_pkt_count = 0;
    ctx->tx_pkt_bytes = 0;

    ctx_ = ctx;
}

//
// @brief Start the block
//
void bk_client_us_nb::start_()
{
    struct client_us_nb_ctx *ctx;

    if (ctx_ == nullptr)
    {
        LOGGER_ERR("Failed to start block: nullptr context");
        return;
    }
    ctx = static_cast<struct client_us_nb_ctx *>(ctx_);

    // Create the client socket
    // TODO: put the socket options in the configuration
    ctx->fd = socket_nb(AF_UNIX, SOCK_STREAM, 0);
    if (ctx->fd == -1)
    {
        LOGGER_ERR("Failed to start block: could not create non-blocking socket [fd=%d]", ctx->fd);
        return;
    }

    // Connect the socket to the server
    connect_();
}

//
// @brief Stop the block
//
void bk_client_us_nb::stop_()
{
    struct client_us_nb_ctx *ctx;

    if (ctx_ == nullptr)
    {
        LOGGER_ERR("Failed to stop block");
        return;
    }
    ctx = static_cast<struct client_us_nb_ctx *>(ctx_);

    LOGGER_INFO("Stop block [ctx=%p]", ctx);

    if (ctx->fd != -1)
    {
        clean_();
    }

    delete ctx;
}

int bk_client_us_nb::tx_(void *vdata)
{
    struct client_us_nb_ctx *ctx;

    if (ctx_ == nullptr)
    {
        LOGGER_ERR("Failed to process TX data: nullptr context");
        return 0;
    }
    ctx = static_cast<struct client_us_nb_ctx *>(ctx_);

    LOGGER_DEBUG("Process TX data [bk_id=%d ; data=%p]", ctx->bk_id, vdata);

    // Update statistics
    ctx->tx_pkt_count++;
    //ctx->tx_pkt_bytes += ?

    // Send to server
    socket_nb_write(ctx->fd, static_cast<const char *>(vdata), SOCKET_READ_SIZE);

    // Drop the buffer
    return 0;
}
