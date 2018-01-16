

#include <stdbool.h>    /* bool */
#include <stdlib.h>     /* NULL */
#include <string.h>     /* memset */
#include <sys/select.h> /* select and associated definitions */

#include "c3qo/logger.h" /* LOGGER */


static fd_set mfd_set;     /* Set of file descriptor */
static int    mfd_set_max; /* Current maximum register fd value */

/**
 * @brief Entry containing a routine to call on a fd
 */
struct mfd_entry
{
        int   fd;
        void (*callback) (int fd);
};

#define MFD_ENTRY_MAX 256
static struct mfd_entry mfd_list[MFD_ENTRY_MAX]; /* List of file descriptors registered */


/**
 * @brief Find the index of a file descriptor in the list
 *
 * @param fd : file descriptor to find
 *
 * @return Index in the list where fd is registered
 *         -1 if not found
 */
static inline int manager_fd_find(int fd)
{
        int i;
        int index = -1;

        for (i = 0; i < MFD_ENTRY_MAX; i++)
        {
                if (mfd_list[i].fd != fd)
                {
                        continue;
                }

                index = i;
                break;
        }

        return index;
}


/**
 * @brief Update the maximum fd value in the list
 */
static inline void manager_fd_update_max()
{
        int i;

        mfd_set_max = -1;

        for (i = 0; i < MFD_ENTRY_MAX; i++)
        {
                if (mfd_list[i].fd > mfd_set_max)
                {
                        mfd_set_max = mfd_list[i].fd;
                }
        }
}


/**
 * @brief Initialize the file descriptor manager
 */
void manager_fd_init()
{
        memset(&mfd_list, -1, sizeof(mfd_list));

        mfd_set_max = -1;

        LOGGER_DEBUG("Initialized file descriptor list");
}


/**
 * @brief Add a file descriptor into the reading list
 */
bool manager_fd_add(int fd, void (*callback) (int fd))
{
        int  i;

        if (callback == NULL)
        {
                LOGGER_WARNING("Cannot add fd=%d with callback=NULL", fd);
                return false;
        }

        /* Look for a free entry */
        i = manager_fd_find(-1);
        if (i != -1)
        {
                mfd_list[i].fd       = fd;
                mfd_list[i].callback = callback;

                FD_SET(fd, &mfd_set);

                /* Update max file descriptor if necessary */
                if (fd > mfd_set_max)
                {
                        mfd_set_max = fd;
                }

                LOGGER_DEBUG("Added fd=%d with callback=%p", fd, callback);
                return true;
        }
        else
        {
                LOGGER_ERR("Could not find room to register fd=%d", fd);
                return false;
        }
}


/**
 * @brief Remove a file descriptor from the reading list
 */
void manager_fd_remove(int fd)
{
        int i;

        i = manager_fd_find(fd);
        if (i != -1)
        {
                mfd_list[i].fd = -1;
                LOGGER_DEBUG("Removed fd=%d from reading list", fd);
        }
        else
        {
                LOGGER_WARNING("Couldn't find fd=%d", fd);
                return;
        }

        FD_CLR(fd, &mfd_set);

        /* If this value was the maximum fd value, we need to refresh it */
        if (fd == mfd_set_max)
        {
                manager_fd_update_max();
        }
}


/**
 * @brief Clean the file descriptor set
 */
void manager_fd_clean()
{
        FD_ZERO(&mfd_set);

        memset(&mfd_list, -1, sizeof(mfd_list));

        mfd_set_max = -1;

        LOGGER_DEBUG("Cleared file descriptor list");
}


/**
 * @brief Verify if a file descriptor is ready for reading
 */
void manager_fd_select()
{
        struct timeval tv;
        int            ret;
        int            c = 0; /* Count of failed select in order not to spam */

        /* Wait up to 10 ms */
        tv.tv_sec  = 0;
        tv.tv_usec = 10000;

        ret = select(mfd_set_max + 1, &mfd_set, NULL, NULL, &tv);
        if (ret == -1)
        {
                if (c < 10)
                {
                        LOGGER_ALERT("file descriptor select failed");
                        c++; /* C++ always come with bad things */
                }
        }
        else if (ret > 0)
        {
                /* There are 'ret' fd ready for reading */
                int i;
                int j = 0; /* Number of executed callbacks */

                for (i = 0; i < MFD_ENTRY_MAX; i++)
                {
                        if ((mfd_list[i].fd == -1) || (FD_ISSET(mfd_list[i].fd, &mfd_set) == 0))
                        {
                                continue;
                        }

                        LOGGER_DEBUG("Data available on fd=%d", mfd_list[i].fd);

                        mfd_list[i].callback(mfd_list[i].fd);
                        j++;

                        if (j == ret)
                        {
                                /* Finished to read */
                                break;
                        }
                }
        }
}


