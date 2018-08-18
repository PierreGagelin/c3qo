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

bk_server_us_nb::bk_server_us_nb(struct manager *mgr) : block(mgr), port_(0), rx_pkt_(0u), tx_pkt_(0u) {}
bk_server_us_nb::~bk_server_us_nb() {}

//
// @brief Callback when a socket is ready for reading
//
// @param fd : file descriptor ready for read
//
void bk_server_us_nb::on_fd_(struct file_desc &fd)
{
    LOGGER_DEBUG("Handle file descriptor callback [bk_id=%d ; fd=%d]", id_, fd.fd);

    if (fd.fd == server_.fd)
    {
        struct file_desc fd_client;
        struct sockaddr_un client;
        socklen_t size;

        // New connection has arrived
        size = sizeof(client);
        fd_client.fd = accept(server_.fd, (struct sockaddr *)&client, &size);
        if (fd_client.fd == -1)
        {
            LOGGER_ERR("Failed to accept new client socket [fd_server=%d ; fd_client=%d]", server_.fd, fd_client.fd);
            return;
        }

        // Non-blocking client
        socket_nb_set(fd_client.fd);

        // Register the fd for event
        fd_client.bk = this;
        fd_client.socket = nullptr;
        fd_client.read = true;
        mgr_->fd_add(fd_client);

        // Keep the new file descriptor
        clients_.insert(fd_client);

        LOGGER_INFO("New client connected [fd_server=%d ; fd_client=%d]", server_.fd, fd_client.fd);
    }
    else
    {
        char buf[SOCKET_READ_SIZE];
        ssize_t ret;

        // Data available from the client
        do
        {
            memset(buf, 0, sizeof(buf));
            ret = socket_nb_read(fd.fd, buf, sizeof(buf));
            if (ret > 0)
            {
                LOGGER_DEBUG("Received data on socket [bk_id=%d ; fd=%d ; buf=%p ; size=%zd]", id_, fd.fd, buf, ret);

                rx_pkt_++;

                // For the moment this is OK because 
                //   - the data flow is synchronous (buf is processed as is)
                //   - only one block is connected
                process_rx_(port_, buf);
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
    server_.fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_.fd == -1)
    {
        LOGGER_ERR("Failed to open server socket [fd=%d]", server_.fd);
        return;
    }

    // Register the file descriptor for reading
    struct file_desc fd_server;
    fd_server.bk = this;
    fd_server.fd = server_.fd;
    fd_server.read = true;
    mgr_->fd_add(fd_server);

    // Set the socket to be NB
    socket_nb_set(server_.fd);

    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sun_family = AF_UNIX;
    strcpy(srv_addr.sun_path, SOCKET_NAME);

    // Close an eventual old socket and bind the new one
    unlink(SOCKET_NAME);
    ret = bind(server_.fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));
    if (ret < 0)
    {
        LOGGER_ERR("Failed to bind server socket [fd=%d]", server_.fd);
        goto err;
    }

    // Listen on the socket with 5 pending connections maximum
    ret = listen(server_.fd, 5);
    if (ret != 0)
    {
        LOGGER_ERR("Failed to listen on server socket [fd=%d]", server_.fd);
        goto err;
    }

    LOGGER_DEBUG("Server socket ready to accept clients [fd=%d]", server_.fd);
    return;

err:
    mgr_->fd_remove(fd_server);
    close(server_.fd);
    server_.fd = -1;
}

//
// @brief Stop the block
//
void bk_server_us_nb::stop_()
{
    LOGGER_INFO("Stop block [bk_id=%d]", id_);

    // Close every client
    for (const auto &fd : clients_)
    {
        LOGGER_DEBUG("Close client connection [fd=%d]", fd.fd);
        mgr_->fd_remove(fd);
        close(fd.fd);
    }
    clients_.clear();

    // Close the server
    LOGGER_DEBUG("Close server connection [fd=%d]", server_.fd);
    mgr_->fd_remove(server_);
    close(server_.fd);

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
        socket_nb_write(fd.fd, static_cast<const char *>(vdata), SOCKET_READ_SIZE);
    }

    // Drop the buffer
    return 0;
}
