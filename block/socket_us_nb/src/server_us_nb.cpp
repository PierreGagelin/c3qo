//
// @brief Implement an AF_UNIX NON-BLOCKING socket
//          - AF_UNIX      : socket domain and SOCK_STREAM type
//          - NON-BLOCKING : return error code instead of blocking
//
// @note us_asnb stand for unix stream non-block
//

// Project headers
#include "c3qo/manager.hpp"

#define SOCKET_NAME "/tmp/server_us_nb"
#define SOCKET_READ_SIZE 256

bk_server_us_nb::bk_server_us_nb(struct manager *mgr) : block(mgr), server_(-1), port_(0), rx_pkt_(0u), tx_pkt_(0u) {}

//
// @brief Callback when a socket is ready for reading
//
// @param fd : file descriptor ready for read
//
static void server_us_nb_handler(void *vctx, int fd, void *socket)
{
    struct bk_server_us_nb *bk;

    (void) socket;

    if (vctx == nullptr)
    {
        LOGGER_ERR("Failed to handle file descriptor callback [fd=%d]", fd);
        return;
    }
    bk = static_cast<struct bk_server_us_nb *>(vctx);

    LOGGER_DEBUG("Handle file descriptor callback [bk_id=%d ; fd=%d]", bk->id_, fd);

    if (fd == bk->server_)
    {
        struct sockaddr_un client;
        socklen_t size;
        int fd_client;

        // New connection has arrived
        size = sizeof(client);
        fd_client = accept(bk->server_, (struct sockaddr *)&client, &size);
        if (fd_client == -1)
        {
            LOGGER_ERR("Failed to accept new client socket [fd_server=%d ; fd_client=%d]", fd, fd_client);
            return;
        }

        // Non-blocking client
        socket_nb_set(fd_client);

        // Keep the new file descriptor
        bk->clients_.insert(fd_client);

        // Register the fd for event
        if (bk->mgr_->fd_add(bk, &server_us_nb_handler, fd_client, nullptr, true) == false)
        {
            LOGGER_ERR("Failed to register callback on new client socket [fd=%d ; callback=%p]", fd_client, &server_us_nb_handler);
            close(fd_client);
            bk->clients_.erase(fd_client);
            return;
        }

        LOGGER_INFO("New client connected [fd=%d ; fd_client=%d]", fd, fd_client);
    }
    else
    {
        char buf[SOCKET_READ_SIZE];
        ssize_t ret;

        // Data available from the client
        do
        {
            memset(buf, 0, sizeof(buf));
            ret = socket_nb_read(fd, buf, sizeof(buf));
            if (ret > 0)
            {
                LOGGER_DEBUG("Received data on socket [bk_id=%d ; fd=%d ; buf=%p ; size=%zd]", bk->id_, fd, buf, ret);

                bk->rx_pkt_++;

                // For the moment this is OK because 
                //   - the data flow is synchronous (buf is processed as is)
                //   - only one block is connected
                bk->process_rx_(bk->port_, buf);
            }
        } while (ret > 0);
    }
}

//
// @brief Start the block
//
void bk_server_us_nb::start_()
{
    struct sockaddr_un srv_addr;
    int ret;

    // Creation of the server socket
    server_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_ == -1)
    {
        LOGGER_ERR("Failed to open server socket [fd=%d]", server_);
        return;
    }

    // Register the file descriptor for reading
    if (mgr_->fd_add(this, &server_us_nb_handler, server_, nullptr, true) == false)
    {
        LOGGER_ERR("Failed to register callback on server socket [fd=%d ; callback=%p]", server_, &server_us_nb_handler);
        goto err;
    }

    // Set the socket to be NB
    socket_nb_set(server_);

    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sun_family = AF_UNIX;
    strcpy(srv_addr.sun_path, SOCKET_NAME);

    // Close an eventual old socket and bind the new one
    unlink(SOCKET_NAME);
    ret = bind(server_, (struct sockaddr *)&srv_addr, sizeof(srv_addr));
    if (ret < 0)
    {
        LOGGER_ERR("Failed to bind server socket [fd=%d]", server_);
        goto err;
    }

    // Listen on the socket with 5 pending connections maximum
    ret = listen(server_, 5);
    if (ret != 0)
    {
        LOGGER_ERR("Failed to listen on server socket [fd=%d]", server_);
        goto err;
    }

    LOGGER_DEBUG("Server socket ready to accept clients [fd=%d ; callback=%p]", server_, &server_us_nb_handler);
    return;

err:
    close(server_);
    server_ = -1;
}

//
// @brief Stop the block
//
void bk_server_us_nb::stop_()
{
    LOGGER_INFO("Stop block [bk_id=%d]", id_);

    // Close every file descriptors
    for (const auto &fd : clients_)
    {
        close(fd);
    }
    clients_.clear();

    // Initialize stats
    rx_pkt_ = 0;
    tx_pkt_ = 0;

    // Remove UNIX socket
    unlink(SOCKET_NAME);
}

//
// @brief Dump statistics of the block in a string
//
// @param buf : String to dump statistics
// @param len : Size of the string
//
// @return Actual size written
//
size_t bk_server_us_nb::get_stats_(char *buf, size_t len)
{
    int ret;
    size_t count;

    LOGGER_DEBUG("Get block statistics [bk_id=%d ; buf=%p ; len=%lu]", id_, buf, len);

    ret = snprintf(buf, len, "%zu", clients_.size());
    if (ret < 0)
    {
        LOGGER_ERR("Failed snprintf [bk_id=%d ; buf=%p ; len=%lu]", id_, buf, len);
        return 0;
    }
    else
    {
        count = static_cast<size_t>(ret);
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

int bk_server_us_nb::tx_(void *vdata)
{
    LOGGER_DEBUG("Process TX data [bk_id=%d ; data=%p]", id_, vdata);

    // Update statistics
    tx_pkt_++;

    // Broadcast to every client
    for (const auto &fd : clients_)
    {
        socket_nb_write(fd, static_cast<const char *>(vdata), SOCKET_READ_SIZE);
    }

    // Drop the buffer
    return 0;
}
