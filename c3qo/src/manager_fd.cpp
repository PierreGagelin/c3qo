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

// Project headers
#include "c3qo/manager_fd.hpp"

//
// @brief Find index of an entry
//
// @return Index of the file descriptor on success, -1 on failure
//
int manager_fd::find(int fd, void *socket)
{
    size_t end;
    size_t i;

    // Verify input
    if ((fd < 0) && (socket == NULL))
    {
        return -1;
    }

    // Look for the file descriptor fd
    end = fd_.size();
    for (i = 0; i < end; i++)
    {
        if ((fd_[i].fd == fd) && (fd_[i].socket == socket))
        {
            return static_cast<int>(i);
        }
    }

    return -1;
}

//
// @brief Add an entry to the polling set
//
// @param callback : function to call when file descriptor or socket is ready
// @param fd       : file descriptor to register
// @param socket   : ZMQ socket to register
// @param read     : register into reading list
//
// @return true on success, false on failure
//
// If both a file descriptor and a socket are set, the socket will prevale when polling
//
bool manager_fd::add(void *ctx, void (*callback)(void *, int, void *), int fd, void *socket, bool read)
{
    struct fd_call new_callback;
    struct fd_call *new_callback_p;
    zmq_pollitem_t new_fd;
    zmq_pollitem_t *new_fd_p;
    int index;

    // Verify user input
    if ((fd < 0) && (socket == NULL))
    {
        LOGGER_WARNING("Cannot add entry: negative file descriptor or NULL socket [fd=%d ; socket=%p]", fd, socket);
        return false;
    }
    if (callback == NULL)
    {
        LOGGER_WARNING("Cannot add entry: NULL callback [fd=%d ; socket=%p ; callback=%p]", fd, socket, callback);
        return false;
    }

    // Update or create a new entry
    index = find(fd, socket);
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

    new_callback_p->ctx = ctx;
    new_callback_p->callback = callback;

    new_fd_p->fd = fd;
    new_fd_p->socket = socket;

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
void manager_fd::remove(int fd, void *socket, bool read)
{
    zmq_pollitem_t *entry;
    int index;
    short flag;

    // Look for the entry
    index = find(fd, socket);
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
        // Remove file descriptor and callback
        auto it_cb = callback_.begin();
        it_cb += index;
        callback_.erase(it_cb);

        auto it_fd = fd_.begin();
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
    ret = zmq_poll(fd_.data(), static_cast<int>(fd_.size()), timeout);
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

        LOGGER_DEBUG("Sockets are ready for I/O [poll_size=%zu ; ready_count=%d]", fd_.size(), ret);

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
                it_cb->callback(it_cb->ctx, it_fd->fd, it_fd->socket);
                count++;
            }

            if (count == ret)
            {
                // There is no need to check further entries
                break;
            }

            ++it_fd;
            ++it_cb;
        }
    }

    return ret;
}
