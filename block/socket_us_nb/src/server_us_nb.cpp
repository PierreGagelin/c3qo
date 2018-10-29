//
// @brief Implement an AF_UNIX NON-BLOCKING socket
//          - AF_UNIX      : socket domain and SOCK_STREAM type
//          - NON-BLOCKING : return error code instead of blocking
//
// @note us_asnb stand for unix stream non-block
//

#define LOGGER_TAG "[block.server_us_nb]"

// Project headers
#include "block/server_us_nb.hpp"
#include "c3qo/manager.hpp"

// C headers
extern "C"
{
#include <sys/un.h>
}

#define SOCKET_NAME "/tmp/server_us_nb"
#define SOCKET_READ_SIZE 256

server_us_nb::server_us_nb(struct manager *mgr) : block(mgr), port_(0), rx_pkt_(0u), tx_pkt_(0u) {}
server_us_nb::~server_us_nb() {}

//
// @brief Callback when a socket is ready for reading
//
// @param fd : file descriptor ready for read
//
void server_us_nb::on_fd_(struct file_desc &fd)
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
        clients_.insert({fd_client.fd, fd_client});

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
void server_us_nb::start_()
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
void server_us_nb::stop_()
{
    LOGGER_INFO("Stop block [bk_id=%d]", id_);

    // Close every client
    for (const auto &fd : clients_)
    {
        LOGGER_DEBUG("Close client connection [fd=%d]", fd.second.fd);
        mgr_->fd_remove(fd.second);
        close(fd.second.fd);
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

int server_us_nb::tx_(void *vdata)
{
    LOGGER_DEBUG("Process TX data [bk_id=%d ; data=%p]", id_, vdata);

    // Update statistics
    tx_pkt_++;

    // Broadcast to every client
    for (const auto &fd : clients_)
    {
        socket_nb_write(fd.second.fd, static_cast<const char *>(vdata), SOCKET_READ_SIZE);
    }

    // Drop the buffer
    return 0;
}

BLOCK_REGISTER(server_us_nb);
