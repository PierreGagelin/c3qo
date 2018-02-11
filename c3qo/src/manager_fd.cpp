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
// @brief Initialize the file descriptor manager
//
void manager_fd::init()
{
    LOGGER_INFO("Initialize manager_fd");

    memset(&list_r, 0, sizeof(list_r));
    memset(&list_w, 0, sizeof(list_w));
    set_max = -1;
}

//
// @brief Update the maximum fd value in the list
//
void manager_fd::update_max()
{
    // Find previous file descriptor managed
    for (int i = set_max; i >= 0; i--)
    {
        if ((list_r[i].callback != NULL) || (list_w[i].callback != NULL))
        {
            set_max = i;
            return;
        }
    }

    // We manage no descriptor
    set_max = -1;
    return;
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
    struct fd_call *list;
    static fd_set *set;

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
        list = list_r;
        set = &set_r;
    }
    else
    {
        list = list_w;
        set = &set_w;
    }

    // Add new file descriptor if not already registered
    if (list[fd].callback == NULL)
    {
        FD_SET(fd, set);

        // Update max file descriptor if necessary
        if (fd > set_max)
        {
            set_max = fd;
        }
    }

    list[fd].callback = callback;
    list[fd].ctx = ctx;

    return true;
}

//
// @brief Remove a file descriptor from the reading list
//
void manager_fd::remove(int fd, bool read)
{
    struct fd_call *list;
    static fd_set *set;

    // Verify user input
    if ((fd >= FD_SETSIZE) || (fd < 0))
    {
        return;
    }

    // Work either on read or write for this file descriptor
    if (read == true)
    {
        list = list_r;
        set = &set_r;
    }
    else
    {
        list = list_w;
        set = &set_w;
    }

    if (list[fd].callback == NULL)
    {
        // Unknown file descriptor, do nothing
        return;
    }

    // Remove from list and set
    list[fd].callback = NULL;
    list[fd].ctx = NULL;
    FD_CLR(fd, set);

    // If this value was the maximum fd value, we need to refresh it
    if (fd == set_max)
    {
        update_max();
    }
}

//
// @brief Clean the file descriptor set
//
void manager_fd::clean()
{
    LOGGER_INFO("Clear file descriptor list");

    FD_ZERO(&set_r);
    FD_ZERO(&set_w);

    memset(&list_r, 0, sizeof(list_r));
    memset(&list_w, 0, sizeof(list_w));

    set_max = -1;
}

//
// @brief Prepare the read and write sets of file descriptor
//
void manager_fd::prepare_set()
{
    FD_ZERO(&set_r);
    FD_ZERO(&set_w);

    for (int fd = 0; fd <= set_max; fd++)
    {
        if (list_r[fd].callback != NULL)
        {
            FD_SET(fd, &set_r);
        }
        if (list_w[fd].callback != NULL)
        {
            FD_SET(fd, &set_w);
        }
    }
}

//
// @brief Verify if a file descriptor is ready for reading
//
// @return Return code of select
//
int manager_fd::select_fd()
{
    struct timeval tv;
    int ret;

    // Wait up to 10ms
    tv.tv_sec = 0;
    tv.tv_usec = 10000;

    prepare_set();

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
            if ((list_r[fd].callback != NULL) && (FD_ISSET(fd, &set_r) != 0))
            {
                // Data ready for reading
                list_r[fd].callback(list_r[fd].ctx, fd);
                j++;
            }
            if ((list_w[fd].callback != NULL) && (FD_ISSET(fd, &set_w) != 0))
            {
                // Data ready for writing
                list_w[fd].callback(list_r[fd].ctx, fd);
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
