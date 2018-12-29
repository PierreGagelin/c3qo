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
#include "engine/manager.hpp"

//
// @brief Find index of an entry
//
// @return Index of the file descriptor on success, -1 on failure
//
int manager::fd_find(int fd, void *socket) const
{
    int i;

    i = 0;
    for (const auto &entry : fd_)
    {
        if ((entry.fd == fd) && (entry.socket == socket))
        {
            return i;
        }
        ++i;
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
bool manager::fd_add(const struct file_desc &fd)
{
    int index;

    // Verify user input
    if (fd.bk == nullptr)
    {
        LOGGER_ERR("Failed to add file descriptor: nullptr block");
        return false;
    }
    if ((fd.fd < 0) && (fd.socket == nullptr))
    {
        LOGGER_ERR("Failed to add file descriptor: wrong values [fd=%d ; socket=%p]", fd.fd, fd.socket);
        return false;
    }

    // Create a new entry
    index = fd_find(fd.fd, fd.socket);
    if (index == -1)
    {
        zmq_pollitem_t new_fd;

        // Initialize the flag
        new_fd.fd = fd.fd;
        new_fd.socket = fd.socket;

        // Store the entry
        callback_.push_back(fd);
        fd_.push_back(new_fd);

        index = fd_.size() - 1;
    }

    // Reset event flags
    auto &poll_item = fd_[static_cast<std::size_t>(index)];
    poll_item.events = 0;
    if (fd.read == true)
    {
        poll_item.events |= ZMQ_POLLIN;
    }
    if (fd.write == true)
    {
        poll_item.events |= ZMQ_POLLOUT;
    }

    return true;
}

//
// @brief Remove an entry
//
void manager::fd_remove(const struct file_desc &fd)
{
    int index;

    // Look for the entry
    index = fd_find(fd.fd, fd.socket);
    if (index == -1)
    {
        // Nothing to do
        return;
    }

    // Remove file descriptor and callback
    callback_.erase(callback_.begin() + index);
    fd_.erase(fd_.begin() + index);
}

//
// @brief Verify if a file descriptor is ready for reading
//
// @return Return code of select
//
int manager::fd_poll()
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
        int count;

        LOGGER_DEBUG("Sockets are ready for I/O [poll_size=%zu ; ready_count=%d]", fd_.size(), ret);

        // There are 'ret' file descriptors ready
        auto it_cb = callback_.begin();
        count = 0;
        for (const auto &it_fd : fd_)
        {
            // Look if the file descriptor is ready
            if ((it_fd.revents & (ZMQ_POLLIN | ZMQ_POLLOUT)) != 0)
            {
                it_cb->bk->on_fd_(*it_cb);
                count++;
            }

            if (count == ret)
            {
                // There is no need to check further entries
                break;
            }

            ++it_cb;
        }
    }

    return ret;
}
