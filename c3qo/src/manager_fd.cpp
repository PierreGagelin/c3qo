//
// @brief Library to manage file descriptors
//
// Basic usage :
//   - register a fd
//   - look for event on the fd
//   - execute callback
//
// Is meant to be a static management of every fd of the process
//

// C++ library headers
#include <cstdlib> // NULL
#include <cstring> // memset, strerror
#include <cerrno>  // errno

// Project headers
#include "c3qo/manager.hpp"
#include "utils/logger.hpp"

//
// @brief Find the index where the file descriptor fd is
//
// @return Index of the file descriptor on success, -1 on failure
//
int manager_fd::find(int fd)
{
    size_t end;
    size_t i;

    // Verify input
    if (fd < 0)
    {
        return -1;
    }

    // Look for the file descriptor fd
    end = fd_.size();
    for (i = 0; i < end; i++)
    {
        if (fd_[i].fd == fd)
        {
            return i;
        }
    }

    return -1;
}

//
// @brief Add a file descriptor
//
// @param fd       : file descriptor
// @param callback : function to call when fd is ready
// @param read     : register into reading list
//
// @return true on success, false on failure
//
bool manager_fd::add(void *ctx, int fd, void (*callback)(void *ctx, int fd), bool read)
{
    struct fd_call new_callback;
    struct fd_call *new_callback_p;
    zmq_pollitem_t new_fd;
    zmq_pollitem_t *new_fd_p;
    int index;

    // Verify user input
    if (callback == NULL)
    {
        LOGGER_WARNING("Cannot add file descriptor without callback [fd=%d ; callback=%p]", fd, callback);
        return false;
    }
    if (fd < 0)
    {
        LOGGER_WARNING("Cannot add negative file descriptor [fd=%d ; callback=%p]", fd, callback);
        return false;
    }

    // Update or create a new entry
    index = find(fd);
    if (index == -1)
    {
        new_callback_p = &new_callback;
        new_fd_p = &new_fd;

        // Initialize the flag
        new_fd_p->events = 0;
    }
    else
    {
        new_callback_p = callback_.data();
        new_fd_p = fd_.data();

        new_callback_p += index;
        new_fd_p += index;
    }

    // Keep the callback context
    new_callback_p->ctx = ctx;
    new_callback_p->callback = callback;

    // Specify we want to poll on the file descriptor rather than the socket
    new_fd_p->fd = fd;
    new_fd_p->socket = NULL;

    // Add a flag to read or write
    new_fd_p->events |= (read == true) ? ZMQ_POLLIN : ZMQ_POLLOUT;

    // Store the new entry
    if (index == -1)
    {
        callback_.push_back(new_callback);
        fd_.push_back(new_fd);
    }

    return true;
}

//
// @brief Remove a flag from the entry. If there are no more flag, the entry is removed
//
void manager_fd::remove(int fd, bool read)
{
    zmq_pollitem_t *entry;
    int index;
    short flag;

    // Look for the entry
    index = find(fd);
    if (index == -1)
    {
        // Nothing to do
        return;
    }
    entry = fd_.data();
    entry += index;

    // Remove the desired flag
    flag = (read == true) ? ZMQ_POLLIN : ZMQ_POLLOUT;
    entry->events &= ~flag;

    // Verify that the entry still has a flag
    if (entry->events == 0)
    {
        std::vector<zmq_pollitem_t>::const_iterator it_fd;
        std::vector<struct fd_call>::const_iterator it_cb;

        // Remove file descriptor and callback
        it_cb = callback_.begin();
        it_cb += index;
        callback_.erase(it_cb);

        it_fd = fd_.begin();
        it_fd += index;
        fd_.erase(it_fd);
    }
}

//
// @brief Verify if a file descriptor is ready for reading
//
// @return Return code of select
//
int manager_fd::poll_fd()
{
    int ret;
    long timeout;

    // Poll sockets for 10ms
    timeout = 10;
    ret = zmq_poll(fd_.data(), fd_.size(), timeout);
    if (ret == -1)
    {
        LOGGER_ERR("Failed to poll socket: %s [errno=%d]", strerror(errno), errno);
        return -1;
    }
    else if (ret == 0)
    {
        // Timeout expired, nothing to do
    }
    else
    {
        std::vector<zmq_pollitem_t>::const_iterator it_fd;
        std::vector<zmq_pollitem_t>::const_iterator end;
        std::vector<struct fd_call>::const_iterator it_cb;
        int count;

        // There are 'ret' file descriptors ready
        it_fd = fd_.begin();
        end = fd_.end();
        it_cb = callback_.begin();
        count = 0;
        while (it_fd != end)
        {
            // Look if the file descriptor is ready
            if ((it_fd->revents & (ZMQ_POLLIN | ZMQ_POLLOUT)) != 0)
            {
                count++;

                if (it_fd->socket != NULL)
                {
                    int fd;
                    size_t fd_len;
                    int opt_ret;

                    fd_len = sizeof(fd);
                    opt_ret = zmq_getsockopt(it_fd->socket, ZMQ_FD, &fd, &fd_len);
                    if (opt_ret == 0)
                    {
                        it_cb->callback(it_cb->ctx, fd);
                    }
                    else
                    {
                        LOGGER_ERR("Failed to retrieve file descriptor from ZMQ socket: %s [errno=%d]", strerror(errno), errno);
                    }
                }
                else
                {
                    it_cb->callback(it_cb->ctx, it_fd->fd);
                }
            }

            if (count == ret)
            {
                break;
            }

            ++it_fd;
            ++it_cb;
        }
    }

    return ret;
}
