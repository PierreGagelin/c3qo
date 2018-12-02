//
// @brief Implement a AF_UNIX NON-BLOCKING client socket
//          - AF_UNIX      : socket domain and SOCK_STREAM type
//          - NON-BLOCKING : return error code instead of blocking
//
// @note us_nb stands for unix stream non-block
//

#define LOGGER_TAG "[block.client_us_nb]"

// Project headers
#include "block/client_us_nb.hpp"
#include "c3qo/manager.hpp"

#define SOCKET_NAME "/tmp/server_us_nb"
#define SOCKET_READ_SIZE 256

client_us_nb::client_us_nb(struct manager *mgr) : block(mgr), port_(0), connected_(false), rx_pkt_(0u), tx_pkt_(0u) {}
client_us_nb::~client_us_nb() {}

//
// @brief Remove the managed file descriptor and close it
//
void client_us_nb::clean_()
{
    LOGGER_INFO("Remove socket from block context [bk_id=%d ; fd=%d]", id_, fd_.fd);

    mgr_->fd_remove(fd_);
    close(fd_.fd);
    fd_.fd = -1;
}

void client_us_nb::on_fd_(struct file_desc &fd)
{
    char buf[SOCKET_READ_SIZE];
    ssize_t ret;

    if (fd.fd != fd_.fd)
    {
        LOGGER_ERR("Failed to handle file descriptor callback: unknown file descriptor [bk_id=%d ; fd_exp=%d ; fd_recv=%d]",
                   id_, fd_.fd, fd.fd);
    }

    LOGGER_DEBUG("Handle file descriptor callback [bk_id=%d ; fd=%d]", id_, fd.fd);

    // Flush the file descriptor
    do
    {
        ret = socket_nb_read(fd_.fd, buf, sizeof(buf));
        if (ret > 0)
        {
            LOGGER_DEBUG("Received data on socket [bk_id=%d ; fd=%d ; buf=%p ; size=%zd]", id_, fd_.fd, buf, ret);

            rx_pkt_++;

            // For the moment this is OK because
            //   - the data flow is synchronous
            //   - only one block is bound
            process_rx_(1, buf);
        }
    } while (ret > 0);
}

//
// @brief Try to connect the socket again. It is more portable to create a new one
//
void client_us_nb::on_timer_(struct timer &)
{
    if (close(fd_.fd) == -1)
    {
        LOGGER_WARNING("Could not close file descriptor properly: I/O might be pending and lost [bk_id=%d ; fd=%d]", id_, fd_.fd);
    }

    // TODO: put the socket options in the configuration
    fd_.fd = socket_nb(AF_UNIX, SOCK_STREAM, 0);
    if (fd_.fd == -1)
    {
        LOGGER_ERR("Failed to retry connection on socket: could not create non-blocking socket [bk_id=%d ; fd=%d]", id_, fd_.fd);
    }

    connect_();
}

//
// @brief Connect to a server
//
void client_us_nb::connect_()
{
    struct sockaddr_un clt_addr;
    int ret;

    // Prepare socket structure
    memset(&clt_addr, 0, sizeof(clt_addr));
    clt_addr.sun_family = AF_UNIX;
    ret = snprintf(clt_addr.sun_path, sizeof(clt_addr.sun_path), SOCKET_NAME);
    if (ret < 0)
    {
        LOGGER_ERR("Failed to connect socket: snprintf error [buf=%p ; size=%lu ; string=%s]",
                   clt_addr.sun_path, sizeof(clt_addr.sun_path), SOCKET_NAME);
        return;
    }
    else if ((static_cast<size_t>(ret)) > sizeof(clt_addr.sun_path))
    {
        LOGGER_ERR("Failed to connect socket: socket name too long [sun_path=%s ; max_size=%lu]",
                   SOCKET_NAME, sizeof(clt_addr.sun_path));
        return;
    }

    // Connect the socket
    if (socket_nb_connect(fd_.fd, (struct sockaddr *)&clt_addr, sizeof(clt_addr)) == true)
    {
        LOGGER_DEBUG("Client socket connected [bk_id=%d ; fd=%d]", id_, fd_.fd);

        // Register the file descriptor for data reception
        fd_.read = true;
        mgr_->fd_add(fd_);
        connected_ = true;
    }
    else
    {
        struct timer tm;

        LOGGER_DEBUG("Couldn't connect: prepare a timer for socket connection retry [bk_id=%d ; fd=%d]", id_, fd_.fd);

        // Prepare a 100ms timer for connection retry
        tm.tid = fd_.fd;
        tm.bk = this;
        tm.arg = nullptr;
        tm.time.tv_sec = 0;
        tm.time.tv_nsec = 100 * 1000 * 1000;
        mgr_->timer_add(tm);
    }
}

//
// @brief Start the block
//
void client_us_nb::start_()
{
    // Create the client socket
    // TODO: put the socket options in the configuration
    fd_.fd = socket_nb(AF_UNIX, SOCK_STREAM, 0);
    if (fd_.fd == -1)
    {
        LOGGER_ERR("Failed to start block: could not create non-blocking socket [bk_id=%d]", id_);
        return;
    }

    fd_.bk = this;
    fd_.socket = nullptr;
    fd_.read = false;
    fd_.write = false;

    // Connect the socket to the server
    connect_();
}

//
// @brief Stop the block
//
void client_us_nb::stop_()
{
    LOGGER_INFO("Stop block [bk_id=%d]", id_);

    if (fd_.fd != -1)
    {
        clean_();
    }
}

int client_us_nb::tx_(void *vdata)
{
    LOGGER_DEBUG("Process TX data [bk_id=%d ; data=%p]", id_, vdata);

    // Update statistics
    tx_pkt_++;

    // Send to server
    socket_nb_write(fd_.fd, static_cast<const char *>(vdata), SOCKET_READ_SIZE);

    // Drop the buffer
    return 0;
}

BLOCK_REGISTER(client_us_nb);
