

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

extern "C" {
#include <stdbool.h>    /* bool */
#include <stdlib.h>     /* NULL */
#include <string.h>     /* memset */
#include <sys/select.h> /* select and associated definitions */
}

#include "c3qo/logger.hpp" /* LOGGER */

namespace manager_fd
{

// Routine to call on a fd
struct fd_call
{
        void (*callback)(int fd);
};

// Sets of file descriptors managed for read and write
fd_set set_r;
fd_set set_w;
int set_max;

// List of registered callbacks for read and write events
struct fd_call list_r[FD_SETSIZE];
struct fd_call list_w[FD_SETSIZE];

/**
 * @brief Initialize the file descriptor manager
 */
void init()
{
        LOGGER_INFO("Initialize manager_fd");

        memset(&manager_fd::list_r, 0, sizeof(manager_fd::list_r));
        memset(&manager_fd::list_w, 0, sizeof(manager_fd::list_w));
        manager_fd::set_max = -1;
}

/**
 * @brief Update the maximum fd value in the list
 */
void update_max()
{
        // Find previous file descriptor managed
        for (int i = manager_fd::set_max; i >= 0; i--)
        {
                if ((manager_fd::list_r[i].callback != NULL) || (manager_fd::list_w[i].callback != NULL))
                {
                        manager_fd::set_max = i;
                        return;
                }
        }

        // We manage no descriptor
        manager_fd::set_max = -1;
        return;
}

/**
 * @brief Add a file descriptor
 *
 * @param fd       : file descriptor
 * @param callback : function to call when fd is ready
 * @param read     : register into reading list
 *
 * @return true on success, false on failure
 */
bool add(int fd, void (*callback)(int fd), bool read)
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
                list = manager_fd::list_r;
                set = &manager_fd::set_r;
        }
        else
        {
                list = manager_fd::list_w;
                set = &manager_fd::set_w;
        }

        // Add new file descriptor if not already registered
        if (list[fd].callback == NULL)
        {
                FD_SET(fd, set);

                // Update max file descriptor if necessary
                if (fd > manager_fd::set_max)
                {
                        manager_fd::set_max = fd;
                }
        }

        list[fd].callback = callback;

        return true;
}

/**
 * @brief Remove a file descriptor from the reading list
 */
void remove(int fd, bool read)
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
                list = manager_fd::list_r;
                set = &manager_fd::set_r;
        }
        else
        {
                list = manager_fd::list_w;
                set = &manager_fd::set_w;
        }

        if (list[fd].callback == NULL)
        {
                /* Unknown file descriptor, do nothing */
                return;
        }

        /* Remove from list and set */
        list[fd].callback = NULL;
        FD_CLR(fd, set);

        /* If this value was the maximum fd value, we need to refresh it */
        if (fd == manager_fd::set_max)
        {
                update_max();
        }
}

/**
 * @brief Clean the file descriptor set
 */
void clean()
{
        LOGGER_INFO("Clear file descriptor list");

        FD_ZERO(&manager_fd::set_r);
        FD_ZERO(&manager_fd::set_w);

        memset(&manager_fd::list_r, 0, sizeof(manager_fd::list_r));
        memset(&manager_fd::list_w, 0, sizeof(manager_fd::list_w));

        manager_fd::set_max = -1;
}

/**
 * @brief Verify if a file descriptor is ready for reading
 *
 * @return Return code of select
 */
int select()
{
        struct timeval tv;
        int ret;

        /* Wait up to 10ms */
        tv.tv_sec = 0;
        tv.tv_usec = 10000;

        ret = select(manager_fd::set_max + 1, &manager_fd::set_r, &manager_fd::set_w, NULL, &tv);
        if (ret == 0)
        {
                /* Nothing to read, select timed out */
        }
        else if (ret == -1)
        {
                LOGGER_ERR("Failed to select on file descriptor set");
        }
        else if (ret > 0)
        {
                int j; /* Number of executed callbacks */

                /* There are 'ret' fd ready for reading */

                j = 0;
                for (int fd = 0; fd <= manager_fd::set_max; fd++)
                {
                        if ((manager_fd::list_r[fd].callback != NULL) && (FD_ISSET(fd, &manager_fd::set_r) != 0))
                        {
                                /* Data ready for reading */
                                manager_fd::list_r[fd].callback(fd);
                                j++;
                        }
                        if ((manager_fd::list_w[fd].callback != NULL) && (FD_ISSET(fd, &manager_fd::set_w) != 0))
                        {
                                /* Data ready for writing */
                                manager_fd::list_w[fd].callback(fd);
                                j++;
                        }

                        if (j >= ret)
                        {
                                /* Finished to read */
                                break;
                        }
                }
        }

        return ret;
}

} // END namespace manager_fd
