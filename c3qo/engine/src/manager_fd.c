

#include <stdbool.h>    /* bool */
#include <stdlib.h>     /* NULL */
#include <string.h>     /* memset */
#include <sys/select.h> /* select and associated definitions */

#include "c3qo/logger.h" /* LOGGER */


static fd_set mfd_set_r;   /* Set of file descriptor for reading */
static fd_set mfd_set_w;   /* Set of file descriptor for writing */
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
static struct mfd_entry mfd_list_r[MFD_ENTRY_MAX]; /* List of file descriptors registered for reading */
static struct mfd_entry mfd_list_w[MFD_ENTRY_MAX]; /* List of file descriptors registered for writing */


/**
 * @brief Find the index of a file descriptor in the list
 *
 * @param fd : file descriptor to find
 *
 * @return Index in the list where fd is registered
 *         -1 if not found
 */
static inline int manager_fd_find(struct mfd_entry *mfd_list, int fd)
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
                if (mfd_list_r[i].fd > mfd_set_max)
                {
                        mfd_set_max = mfd_list_r[i].fd;
                }
                if (mfd_list_w[i].fd > mfd_set_max)
                {
                        mfd_set_max = mfd_list_w[i].fd;
                }
        }
}


/**
 * @brief Initialize the file descriptor manager
 */
void manager_fd_init()
{
        LOGGER_DEBUG("Initialize manager_fd");

        memset(&mfd_list_r, -1, sizeof(mfd_list_r));
        memset(&mfd_list_w, -1, sizeof(mfd_list_w));
        mfd_set_max = -1;
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
bool manager_fd_add(int fd, void (*callback) (int fd), bool read)
{
        struct mfd_entry *mfd_list;
        static fd_set    *mfd_set;
        int              i;

        if (read == true)
        {
                mfd_list = mfd_list_r;
                mfd_set  = &mfd_set_r;
        }
        else
        {
                mfd_list = mfd_list_w;
                mfd_set  = &mfd_set_w;
        }

        if (callback == NULL)
        {
                LOGGER_WARNING("Cannot add file descriptor without callback [fd=%d ; callback=%p]", fd, callback);
                return false;
        }

        /* Look for a free entry */
        i = manager_fd_find(mfd_list, -1);
        if (i != -1)
        {
                mfd_list[i].fd       = fd;
                mfd_list[i].callback = callback;

                FD_SET(fd, mfd_set);

                /* Update max file descriptor if necessary */
                if (fd > mfd_set_max)
                {
                        mfd_set_max = fd;
                }

                return true;
        }
        else
        {
                LOGGER_ERR("Failed to find room to add file descriptor [fd=%d ; callback=%p]", fd, callback);
                return false;
        }
}


/**
 * @brief Remove a file descriptor from the reading list
 */
void manager_fd_remove(int fd, bool read)
{
        struct mfd_entry *mfd_list;
        static fd_set    *mfd_set;
        int               i;

        if (read == true)
        {
                mfd_list = mfd_list_r;
                mfd_set  = &mfd_set_r;
        }
        else
        {
                mfd_list = mfd_list_w;
                mfd_set  = &mfd_set_w;
        }

        i = manager_fd_find(mfd_list, fd);
        if (i == -1)
        {
                /* Unknown file descriptor, do nothing */
                return;
        }

        /* Remove from list and set */
        mfd_list[i].fd = -1;
        FD_CLR(fd, mfd_set);

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
        LOGGER_DEBUG("Clear file descriptor list");

        FD_ZERO(&mfd_set_r);
        FD_ZERO(&mfd_set_w);

        memset(&mfd_list_r, -1, sizeof(mfd_list_r));
        memset(&mfd_list_w, -1, sizeof(mfd_list_w));

        mfd_set_max = -1;
}


/**
 * @brief Verify if a file descriptor is ready for reading
 *
 * @return Return code of select
 */
int manager_fd_select()
{
        struct timeval tv;
        int            ret;

        /* Wait up to 10ms */
        tv.tv_sec  = 0;
        tv.tv_usec = 10000;

        ret = select(mfd_set_max + 1, &mfd_set_r, &mfd_set_w, NULL, &tv);
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
                int i;
                int j; /* Number of executed callbacks */

                /* There are 'ret' fd ready for reading */

                j = 0;
                for (i = 0; i < MFD_ENTRY_MAX; i++)
                {
                        if ((mfd_list_r[i].fd != -1) && (FD_ISSET(mfd_list_r[i].fd, &mfd_set_r) != 0))
                        {
                                /* Data ready for reading */
                                mfd_list_r[i].callback(mfd_list_r[i].fd);
                                j++;
                        }
                        if ((mfd_list_w[i].fd != -1) && (FD_ISSET(mfd_list_w[i].fd, &mfd_set_w) != 0))
                        {
                                /* Data ready for writing */
                                mfd_list_w[i].callback(mfd_list_w[i].fd);
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


