//
// @brief API to manage socket
//

#define LOGGER_TAG "[lib.socket]"

// Project headers
#include "utils/socket.hpp"
#include "utils/logger.hpp"

//
// @brief Create a socket and set it to be non-blocking
//
int socket_nb(int domain, int type, int protocol)
{
    int fd;

    // Create the client socket
    fd = socket(domain, type, protocol);
    if (fd == -1)
    {
        return -1;
    }

    // Set the socket to be non-blocking
    socket_nb_set(fd);

    return fd;
}

//
// @brief Set the file descriptor to be non-blocking
//
void socket_nb_set(int fd)
{
    int flags;

    flags = fcntl(fd, F_GETFL, 0);
    flags |= O_NONBLOCK;

    fcntl(fd, F_SETOWN, getpid());
    fcntl(fd, F_SETFL, flags);
}

//
// @brief Non-blocking write to the file descriptor
//
ssize_t socket_nb_write(int fd, const char *buff, size_t size)
{
    ssize_t ret;

    ret = write(fd, buff, size);
    if (ret >= 0)
    {
        // Write succeed
        return ret;
    }

    // Catch a would-have-block error
    switch (errno)
    {
    case EAGAIN:
    {
        LOGGER_DEBUG("Cannot write on non-blocking socket: %s [fd=%d ; errno=%d]", strerror(errno), fd, errno);
        break;
    }
    default:
    {
        LOGGER_ERR("Failed to write on non-blocking socket: %s [fd=%d ; errno=%d]", strerror(errno), fd, errno);
        break;
    }
    }

    return ret;
}

//
// @brief Non-blocking read to the file descriptor
//
ssize_t socket_nb_read(int fd, char *buff, size_t size)
{
    ssize_t ret;

    ret = read(fd, buff, size);
    if (ret >= 0)
    {
        // Read succeed
        return ret;
    }

    // Catch a would-have-block error
    switch (errno)
    {
    case EAGAIN:
    {
        LOGGER_DEBUG("Cannot read on non-blocking socket: %s [fd=%d ; errno=%d]", strerror(errno), fd, errno);
        break;
    }
    default:
    {
        LOGGER_ERR("Failed to read on non-blocking socket: %s [fd=%d ; errno=%d]", strerror(errno), fd, errno);
        break;
    }
    }

    return ret;
}

//
// @brief Check if a socket is connected
//
bool socket_nb_connect_check(int fd)
{
    socklen_t len;
    int optval;

    // Verify connection status
    len = sizeof(optval);
    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, (void *)(&optval), &len) != 0)
    {
        LOGGER_ERR("Failed to check socket connection status: getsockopt failed [fd=%d]", fd);
        return false;
    }

    if (optval != 0)
    {
        LOGGER_DEBUG("Check socket connection status: socket not connected [fd=%d]", fd);
        return false;
    }

    LOGGER_DEBUG("Check socket connection status: socket connected [fd=%d]", fd);

    return true;
}

//
// @brief Connect in a non-blocking way
//
// @param fd : socket that shall be non-blocking
//
// @return Several codes :
//           - -1 : failure
//           - 0  : success
//           - 1  : need to call getsockopt
//           - 2  : need to call connect again
//
int socket_nb_connect(int fd, const struct sockaddr *addr, socklen_t len)
{
    int ret;

    ret = connect(fd, addr, len);
    if (ret == 0)
    {
        LOGGER_DEBUG("Connected non-blocking socket [fd=%d]", fd);
        return 0;
    }

    switch (errno)
    {
    case EISCONN:
        // Socket already connected, nothing to do
        return 0;

    case EINPROGRESS:
    case EALREADY:
        // EINPROGRESS: server is listening but not answering, waiting for getsockopt
        // EALREADY   : socket was already in EINPROGRESS, waiting for getsockopt
        LOGGER_DEBUG("Cannot connect non-blocking socket: %s [fd=%d ; errno=%d]", strerror(errno), fd, errno);
        return 1;

    case ECONNREFUSED:
        LOGGER_DEBUG("Cannot connect non-blocking socket: %s [fd=%d ; errno=%d]", strerror(errno), fd, errno);
        return 2;

    case EAGAIN:
        LOGGER_DEBUG("Cannot connect non-blocking socket: %s [fd=%d ; errno=%d]", strerror(errno), fd, errno);
        return 2;

    case EADDRNOTAVAIL:
    default:
        // La pauvre socket n'a pas de travail, dommage pour elle
        LOGGER_ERR("Failed to connect non-blocking socket: %s [fd=%d ; errno=%d]", strerror(errno), fd, errno);
        return -1;
    }
}

void c3qo_zmq_msg_del(std::vector<struct c3qo_zmq_part> &msg)
{
    for (const auto &it: msg)
    {
        delete[] it.data;
    }
}

//
// @brief Receive data from a ZMQ socket
//
void socket_zmq_read(void *socket, std::vector<struct c3qo_zmq_part> &msg, int flags)
{
    struct c3qo_zmq_part msg_part;

    while (true)
    {
        zmq_msg_t part;
        int ret;

        // Receive a message
        zmq_msg_init(&part);

        ret = zmq_msg_recv(&part, socket, flags);
        if (ret <= 0)
        {
            LOGGER_ERR("Failed to receive data from ZMQ socket: %s [errno=%d]", strerror(errno), errno);
            break;
        }

        // Copy the message
        // - add a NULL byte in case it's a string
        // - do not count it in the message length
        msg_part.len = zmq_msg_size(&part);
        msg_part.data = new char[msg_part.len + 1];
        memcpy(msg_part.data, zmq_msg_data(&part), msg_part.len);
        msg_part.data[msg_part.len] = '\0';

        msg.push_back(msg_part);

        // Look if there is another part to come
        ret = zmq_msg_more(&part);

        zmq_msg_close(&part);
        if (ret != 1)
        {
            break;
        }
    }
}

//
// @brief Send a ZMQ multi-part message
//
// @return True on success
//
bool socket_zmq_write(void *socket, std::vector<struct c3qo_zmq_part> &msg, int flags)
{
    flags |= ZMQ_SNDMORE;
    for (size_t idx = 0u; idx < msg.size(); ++idx)
    {
        zmq_msg_t message;
        int rc;

        zmq_msg_init_size(&message, msg[idx].len);
        memcpy(zmq_msg_data(&message), msg[idx].data, msg[idx].len);

        if (idx == msg.size() - 1)
        {
            flags &= ~ZMQ_SNDMORE;
        }
        rc = zmq_msg_send(&message, socket, flags);
        if (rc == -1)
        {
            LOGGER_ERR("Failed to write ZMQ message: %s [errno=%d ; len=%zu ; data=%s]",
                       strerror(errno), errno, msg[idx].len, msg[idx].data);
            zmq_msg_close(&message);
            return false;
        }
    }
    return true;
}

//
// @brief Return an event sniffed on monitored ZMQ socket
//
// Beware that read operation is intentionally blocking
//
int socket_zmq_get_event(void *monitor)
{
    std::vector<struct c3qo_zmq_part> msg;
    uint16_t event;

    // Retrieve event in the first message part
    socket_zmq_read(monitor, msg, 0);
    if (msg.size() != 2u)
    {
        event = 0xffff;
    }
    else
    {
        event = *(uint16_t *)(msg[0].data);
    }

    c3qo_zmq_msg_del(msg);

    return event;
}
