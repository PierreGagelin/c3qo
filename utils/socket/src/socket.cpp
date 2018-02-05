//
// @brief API to manage socket
//

// C++ library headers
#include <cerrno>

// System library headers
extern "C" {
#include <fcntl.h>
#include <unistd.h>
}

// Project headers
#include "utils/socket.hpp"
#include "utils/logger.hpp"

//
// @brief Set the file descriptor to be NON-BLOCKING
//
void c3qo_socket_set_nb(int fd)
{
    int flags;

    flags = fcntl(fd, F_GETFL, 0);
    flags |= O_NONBLOCK;

    fcntl(fd, F_SETOWN, getpid()); // WARN: not POSIX
    fcntl(fd, F_SETFL, flags);     // WARN: not POSIX
}

//
// @brief Non-blocking write to the file descriptor
//
ssize_t c3qo_socket_write_nb(int fd, const char *buff, size_t size)
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
        LOGGER_DEBUG("Socket not ready to send data [fd=%d]", fd);
        break;
    }
    default:
    {
        LOGGER_ERR("Failed non-blocking write on socket [fd=%d]", fd);
        break;
    }
    }

    return ret;
}

//
// @brief Non-blocking read to the file descriptor
//
ssize_t c3qo_socket_read_nb(int fd, char *buff, size_t size)
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
        LOGGER_DEBUG("Socket not ready to receive data [fd=%d]", fd);
        break;
    }
    default:
    {
        LOGGER_ERR("Failed non-blocking read on socket [fd=%d]", fd);
        break;
    }
    }

    return ret;
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
int c3qo_socket_connect_nb(int fd, const struct sockaddr *addr, socklen_t len)
{
    int ret;

    ret = connect(fd, addr, len);
    if (ret == 0)
    {
        // Successfull connection
        return 0;
    }

    switch (errno)
    {
    case EISCONN:
        // Socket already connected, nothing to do
        return 0;
    case EINPROGRESS:
    case EALREADY:
        //
        // EINPROGRESS : server is listening but not answering, waiting for getsockopt
        // EALREADY    : socket was already in EINPROGRESS, waiting for getsockopt
        //
        return 1;
    case ECONNREFUSED:
        // No one listening on the socket
        return 2;
    case EADDRNOTAVAIL:
    default:
        // La pauvre socket n'a pas de travail, dommage pour elle
        LOGGER_ERR("Failed to handle errno on socket connect [fd=%d ; connect=%d ; errno=%d]", fd, ret, errno);
        return -1;
    }
}