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
#include <cstring> // memset

// Project headers
#include "c3qo/manager_fd.hpp"
#include "utils/logger.hpp"

// One static instance of timer manager
class manager_fd m_fd;

//
// @brief Constructor
//
// We size vector to maximum, it allows to have a small binary footprint
//
manager_fd::manager_fd()
{
    struct fd_call def;

    def.ctx = NULL;
    def.callback = NULL;

    list_r_.assign(FD_SETSIZE, def);
    list_w_.assign(FD_SETSIZE, def);
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
    // Verify user input
    if (callback == NULL)
    {
        LOGGER_WARNING("Cannot add file descriptor without callback [fd=%d ; callback=%p]", fd, callback);
        return false;
    }
    if (fd >= FD_SETSIZE)
    {
        LOGGER_CRIT("File descriptor value too high for select, need to use poll [fd=%d ; callback=%p]", fd, callback);
        return false;
    }
    if (fd < 0)
    {
        LOGGER_WARNING("Cannot add negative file descriptor [fd=%d ; callback=%p]", fd, callback);
        return false;
    }

    // Work either on read or write for this file descriptor
    if (read == true)
    {
        list_r_[fd].ctx = ctx;
        list_r_[fd].callback = callback;
    }
    else
    {
        list_w_[fd].ctx = ctx;
        list_w_[fd].callback = callback;
    }

    return true;
}

//
// @brief Remove a file descriptor from the reading list
//
void manager_fd::remove(int fd, bool read)
{
    // Verify user input
    if ((fd >= FD_SETSIZE) || (fd < 0))
    {
        return;
    }

    // Work either on read or write for this file descriptor
    if (read == true)
    {
        list_r_[fd].callback = NULL;
        list_r_[fd].ctx = NULL;
    }
    else
    {
        list_w_[fd].callback = NULL;
        list_w_[fd].ctx = NULL;
    }
}

//
// @brief Prepare the read and write sets of file descriptor
//
// @param set_r : file descriptor set to fill for read
// @param set_w : file descriptor set to fill for write
//
// @return maximum file descriptor value
//
int manager_fd::prepare_set(fd_set *set_r, fd_set *set_w)
{
    int max;

    FD_ZERO(set_r);
    FD_ZERO(set_w);

    max = 0;
    for (int fd = 0; fd < FD_SETSIZE; fd++)
    {
        if (list_r_[fd].callback != NULL)
        {
            FD_SET(fd, set_r);

            if (fd > max)
            {
                max = fd;
            }
        }
        if (list_w_[fd].callback != NULL)
        {
            FD_SET(fd, set_w);

            if (fd > max)
            {
                max = fd;
            }
        }
    }

    return max;
}

//
// @brief Verify if a file descriptor is ready for reading
//
// @return Return code of select
//
int manager_fd::select_fd()
{
    struct timeval tv;
    fd_set set_r;
    fd_set set_w;
    int set_max;
    int ret;

    // Wait up to 10ms
    tv.tv_sec = 0;
    tv.tv_usec = 10 * 1000;

    set_max = prepare_set(&set_r, &set_w);

    ret = select(set_max + 1, &set_r, &set_w, NULL, &tv);
    if (ret == 0)
    {
        // Nothing to read, select timed out
    }
    else if (ret == -1)
    {
        LOGGER_ERR("Failed to select on file descriptor set");
    }
    else if (ret > 0)
    {
        int j; // Number of executed callbacks

        // There are 'ret' fd ready for reading

        j = 0;
        for (int fd = 0; fd <= set_max; fd++)
        {
            if ((FD_ISSET(fd, &set_r) != 0) && (list_r_[fd].callback != NULL))
            {
                // Data ready for reading
                list_r_[fd].callback(list_r_[fd].ctx, fd);
                j++;
            }
            if ((FD_ISSET(fd, &set_w) != 0) && (list_w_[fd].callback != NULL))
            {
                // Data ready for writing
                list_w_[fd].callback(list_r_[fd].ctx, fd);
                j++;
            }

            if (j >= ret)
            {
                // Finished to read
                break;
            }
        }
    }

    return ret;
}
