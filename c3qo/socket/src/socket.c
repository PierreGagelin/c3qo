/**
 * @brief API to manage socket
 */


/* WARN: non-POSIX */
#define _GNU_SOURCE


#include "c3qo/socket.h"
#include "c3qo/logger.h"

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/signal.h>
#include <unistd.h>


/**
 * @brief Set the file descriptor to be NON-BLOCKING
 */
void c3qo_socket_set_nb(int fd)
{
        int flags;

        flags  = fcntl(fd, F_GETFL, 0);
        flags |= O_NONBLOCK;

        fcntl(fd, F_SETOWN, getpid()); /* WARN: not POSIX */
        fcntl(fd, F_SETFL, flags);     /* WARN: not POSIX */
}


/**
 * @brief Non-blocking write to the file descriptor
 */
ssize_t c3qo_socket_write_nb(int fd, const char *buff, size_t size)
{
        ssize_t ret;

        /* Try to write */
        errno = 0;
        ret = write(fd, buff, size);

        /* Catch a would-have-block error */
        switch (errno)
        {
        case 0:
        {
                /* no error */
                break;
        }
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


/**
 * @brief Non-blocking read to the file descriptor
 */
ssize_t c3qo_socket_read_nb(int fd, char *buff, size_t size)
{
        ssize_t ret;

        /* Try to read */
        errno = 0;
        ret = read(fd, buff, size); 

        /* Catch a would-have-block error */
        switch (errno)
        {
        case 0:
        {
                /* no error */
                break;
        }
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


