//
// @brief Implement a AF_UNIX NON-BLOCKING client socket
//          - AF_UNIX      : socket domain and SOCK_STREAM type
//          - NON-BLOCKING : return error code instead of blocking
//
// @note us_nb stands for unix stream non-block
//

// Project headers
#include "c3qo/manager.hpp"

#define SOCKET_NAME "/tmp/server_us_nb"
#define SOCKET_READ_SIZE 256

bk_client_us_nb::bk_client_us_nb(struct manager *mgr) : block(mgr), port_(0), fd_(-1), connected_(false), rx_pkt_(0u), tx_pkt_(0u) {}
bk_client_us_nb::~bk_client_us_nb() {}

//
// @brief Remove the managed file descriptor and close it
//
void bk_client_us_nb::clean_()
{
    LOGGER_INFO("Remove socket from block context [bk_id=%d ; fd=%d]", id_, fd_);

    mgr_->fd_remove(fd_, nullptr, true);
    mgr_->fd_remove(fd_, nullptr, false);
    close(fd_);
    fd_ = -1;
}

//
// @brief Callback function when data is received
//
static void client_us_nb_callback(void *vctx, int fd, void *socket)
{
    struct bk_client_us_nb *bk;
    char buf[SOCKET_READ_SIZE];
    ssize_t ret;

    (void) socket;

    // Verify input
    if (vctx == nullptr)
    {
        LOGGER_ERR("Failed to handle file descriptor callback: nullptr context [fd=%d]", fd);
        return;
    }
    bk = static_cast<struct bk_client_us_nb *>(vctx);
    if (fd != bk->fd_)
    {
        LOGGER_ERR("Failed to handle file descriptor callback: unknown file descriptor [bk_id=%d ; fd_exp=%d ; fd_recv=%d]", bk->id_, bk->fd_, fd);
    }

    LOGGER_DEBUG("Handle file descriptor callback [bk_id=%d ; fd=%d]", bk->id_, fd);

    // Flush the file descriptor
    do
    {
        ret = socket_nb_read(bk->fd_, buf, sizeof(buf));
        if (ret > 0)
        {
            LOGGER_DEBUG("Received data on socket [bk_id=%d ; fd=%d ; buf=%p ; size=%zd]", bk->id_, bk->fd_, buf, ret);

            bk->rx_pkt_++;

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
    struct bk_client_us_nb *bk;

    (void) socket;

    // Verify input
    if (vctx == nullptr)
    {
        LOGGER_ERR("Failed to check socket connection status: nullptr context [fd=%d]", fd);
        return;
    }
    bk = static_cast<struct bk_client_us_nb *>(vctx);
    if (fd != bk->fd_)
    {
        LOGGER_ERR("Failed to check socket connection status: unknown file descriptor [bk_id=%d ; fd_exp=%d ; fd_recv=%d]", bk->id_, bk->fd_, fd);
    }

    if (socket_nb_connect_check(bk->fd_) == true)
    {
        // Socket is connected, no need to look for write occasion anymore
        bk->connected_ = true;
        bk->mgr_->fd_remove(bk->fd_, nullptr, false);
        bk->mgr_->fd_add(bk, &client_us_nb_callback, bk->fd_, nullptr, true);
    }
}

//
// @brief Try to connect the socket again. It is more portable to create a new one
//
void bk_client_us_nb::on_timer_(struct timer &)
{
    if (close(fd_) == -1)
    {
        LOGGER_WARNING("Could not close file descriptor properly: I/O might be pending and lost [bk_id=%d ; fd=%d]", id_, fd_);
    }

    // TODO: put the socket options in the configuration
    fd_ = socket_nb(AF_UNIX, SOCK_STREAM, 0);
    if (fd_ == -1)
    {
        LOGGER_ERR("Failed to retry connection on socket: could not create non-blocking socket [bk_id=%d ; fd=%d]", id_, fd_);
    }

    connect_();
}

//
// @brief Connect to a server
//
void bk_client_us_nb::connect_()
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
    else if ((static_cast<size_t>(ret)) > sizeof(clt_addr.sun_path))
    {
        LOGGER_ERR("Failed to connect socket: socket name too long [sun_path=%s ; max_size=%lu]", SOCKET_NAME, sizeof(clt_addr.sun_path));
        return;
    }

    // Connect the socket
    ret = socket_nb_connect(fd_, (struct sockaddr *)&clt_addr, sizeof(clt_addr));
    switch (ret)
    {
    case 1:
        // Connection in progress, register file descriptor for writing to check the connection when it's ready
        mgr_->fd_add(this, &client_us_nb_connect_check, fd_, nullptr, false);
        break;

    case -1:
    case 2:
        struct timer tm;

        LOGGER_DEBUG("Prepare a timer for socket connection retry [bk_id=%d ; fd=%d]", id_, fd_);

        // Prepare a 100ms timer for connection retry
        tm.tid = fd_;
        tm.bk = this;
        tm.arg = nullptr;
        tm.time.tv_sec = 0;
        tm.time.tv_nsec = 100 * 1000 * 1000;
        mgr_->timer_add(tm);
        break;

    case 0:
        // Success: register the file descriptor with a callback for data reception
        if (mgr_->fd_add(this, &client_us_nb_callback, fd_, nullptr, true) == false)
        {
            LOGGER_ERR("Failed to register callback on client socket [fd=%d ; callback=%p]", fd_, &client_us_nb_callback);
            clean_();
        }
        else
        {
            LOGGER_DEBUG("Registered callback on client socket [fd=%d ; callback=%p]", fd_, &client_us_nb_callback);
            connected_ = true;
        }
        break;
    }
}

//
// @brief Start the block
//
void bk_client_us_nb::start_()
{
    // Create the client socket
    // TODO: put the socket options in the configuration
    fd_ = socket_nb(AF_UNIX, SOCK_STREAM, 0);
    if (fd_ == -1)
    {
        LOGGER_ERR("Failed to start block: could not create non-blocking socket [fd=%d]", fd_);
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
    LOGGER_INFO("Stop block [bk_id=%d]", id_);

    if (fd_ != -1)
    {
        clean_();
    }
}

int bk_client_us_nb::tx_(void *vdata)
{
    LOGGER_DEBUG("Process TX data [bk_id=%d ; data=%p]", id_, vdata);

    // Update statistics
    tx_pkt_++;

    // Send to server
    socket_nb_write(fd_, static_cast<const char *>(vdata), SOCKET_READ_SIZE);

    // Drop the buffer
    return 0;
}
