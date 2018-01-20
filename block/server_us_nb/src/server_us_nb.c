/**
 * @brief Implement an AF_UNIX NON-BLOCKING socket
 *          - AF_UNIX      : socket domain and SOCK_STREAM type
 *          - NON-BLOCKING : return error code instead of blocking
 *
 * @note us_asnb stand for unix stream non-block
 */


#include <unistd.h>     /* close, unlink */
#include <stdio.h>      /* snprintf */
#include <sys/types.h>  /* listen */
#include <sys/un.h>     /* sockaddr_un */
#include <sys/socket.h> /* socket, listen */

#include "c3qo/block.h"
#include "c3qo/logger.h"
#include "c3qo/manager_fd.h"
#include "c3qo/socket.h"


#define SOCKET_FD_MAX    64 /* Maximum number of file descriptors */
#define SOCKET_READ_SIZE 256
#define SOCKET_NAME      "/tmp/server_us_nb"


/* Context of the block */
struct server_us_nb_ctx
{
        int fd[SOCKET_FD_MAX]; /* File descriptors of the socket */
        int fd_count;          /* Number of fd in use */
};
struct server_us_nb_ctx ctx_s;


/* Statistics */
unsigned int server_us_nb_count;
ssize_t      server_us_nb_bytes;


static inline int server_us_nb_fd_find(int fd)
{
        int i;

        for (i = 0; i < SOCKET_FD_MAX; i++)
        {
                if (ctx_s.fd[i] != fd)
                {
                        continue;
                }
                
                return i;
        }

        return -1;
}


/**
 * @brief Add a file descriptor to the context
 *
 * @return -1 on failure, index of input on success
 */
static int server_us_nb_fd_add(int fd)
{
        int i;

        c3qo_socket_set_nb(fd);

        if (ctx_s.fd[ctx_s.fd_count] != -1)
        {
                /* First easy try */
                i = ctx_s.fd_count;
        }
        else
        {
                /* Easy try failed, looking for first available */
                i = server_us_nb_fd_find(-1);
        }

        if (i != -1)
        {
                LOGGER_DEBUG("Add file descriptor to block server_us_nb [fd=%d ; fd_count_old=%d ; fd_count_new=%d]", fd, ctx_s.fd_count, ctx_s.fd_count + 1);

                ctx_s.fd[i] = fd;
                ctx_s.fd_count++;
        }
        else
        {
                LOGGER_ERR("Failed to find room for new file descriptor [fd=%d ; fd_count=%d]", fd, ctx_s.fd_count);
        }

        return i;
}


/**
 * @brief Removes a file descriptor
 *
 * @param i : index where to find the file descriptor
 */
static void server_us_nb_remove(int i)
{
        /* Check bounds and coherency */
        if ((i < 0) || (i >= SOCKET_FD_MAX) || (ctx_s.fd[i] == -1))
        {
                return;
        }

        LOGGER_DEBUG("Remove file descriptor from block server_us_nb [fd=%d ; fd_count_old=%d ; fd_count_new=%d]", ctx_s.fd[i], ctx_s.fd_count, ctx_s.fd_count - 1);

        manager_fd_remove(ctx_s.fd[i], true);
        close(ctx_s.fd[i]);
        ctx_s.fd[i] = -1;
        ctx_s.fd_count--;
}


/**
 * @brief Removes a file descriptor
 *
 * @param fd : file descriptor
 */
static inline void server_us_nb_remove_fd(int fd)
{
        int i;

        i = server_us_nb_fd_find(fd);

        if (i != -1)
        {
                server_us_nb_remove(i);
        }
}


/**
 * @brief Flush a file descriptor
 */
static void server_us_nb_flush_fd(int fd)
{
        ssize_t ret;
        char    buff[SOCKET_READ_SIZE];

        do
        {
                memset(buff, 0, sizeof(buff));
                ret = c3qo_socket_read_nb(fd, buff, sizeof(buff));
                if (ret == 0)
                {
                        server_us_nb_count += 1;
                        server_us_nb_bytes += ret;
                }
        } while (ret == 0);
}


/**
 * @brief Callback when a socket is ready for reading
 *
 * @param fd : file descriptor ready for read
 */
static void server_us_nb_handler(int fd)
{
        LOGGER_DEBUG("Data available on socket [fd=%d]", fd);
        if (fd == ctx_s.fd[0])
        {
                struct sockaddr_un client;
                socklen_t          size;
                int                fd_client;

                /* New connection has arrived */
                size = sizeof(client);
                fd_client = accept(ctx_s.fd[0], (struct sockaddr *) &client, &size);
                if (fd_client == -1)
                {
                        LOGGER_ERR("Failed to accept new client socket [fd_server=%d ; fd_client=%d]", fd, fd_client);
                        return;
                }

                /* Keep the new file descriptor */
                if (server_us_nb_fd_add(fd_client) == -1)
                {
                        LOGGER_ERR("Failed to add new client socket [fd=%d]", fd_client);
                        server_us_nb_remove_fd(fd_client);
                        return;
                }

                /* Register the fd for event */
                if (manager_fd_add(fd_client, &server_us_nb_handler, true) == false)
                {
                        LOGGER_ERR("Failed to register callback on new client socket [fd=%d ; callback=%p]", fd_client, &server_us_nb_handler);
                        server_us_nb_remove_fd(fd_client);
                        return;
                }
                else
                {
                        LOGGER_DEBUG("Registered callback on new client socket [fd=%d ; callback=%p]", fd_client, &server_us_nb_handler);
                }
        }
        else
        {
                /* Data available from the client */
                server_us_nb_flush_fd(fd);

                LOGGER_DEBUG("Statistics of server_us_nb [count=%u ; bytes=%ld]", server_us_nb_count, server_us_nb_bytes);
        }
}


/**
 * @brief Initialize the block
 */
static void server_us_nb_init()
{
        LOGGER_INFO("Initialize block server_us_nb");

        /* Initialize context */
        memset(&ctx_s, -1, sizeof(ctx_s));
        ctx_s.fd_count = 0;

        /* Initialize stats */
        server_us_nb_count = 0;
        server_us_nb_bytes = 0;

        /* Remove UNIX socket */
        unlink(SOCKET_NAME);
}


/**
 * @brief Start the block
 */
static void server_us_nb_start()
{
        struct sockaddr_un srv_addr;
        int                ret;

        LOGGER_INFO("Start block server_us_nb");

        /* Creation of the server socket */
        ctx_s.fd[0] = socket(AF_UNIX, SOCK_STREAM, 0);
        ctx_s.fd_count++;
        if (ctx_s.fd[0] == -1)
        {
                LOGGER_ERR("Failed to open server socket [fd=%d]", ctx_s.fd[0]);
                return;
        }

        /* Register the file descriptor for reading */
        if (manager_fd_add(ctx_s.fd[0], &server_us_nb_handler, true) == false)
        {
                LOGGER_ERR("Failed to register callback on server socket [fd=%d ; callback=%p]", ctx_s.fd[0], &server_us_nb_handler);
                server_us_nb_remove_fd(ctx_s.fd[0]);
                return;
        }

        /* Set the socket to be NB */
        c3qo_socket_set_nb(ctx_s.fd[0]);

        memset(&srv_addr, 0, sizeof(srv_addr));
        srv_addr.sun_family = AF_UNIX;
        strcpy(srv_addr.sun_path, SOCKET_NAME);

        /* Close an eventual old socket and bind the new one */
        unlink(SOCKET_NAME);
        ret = bind(ctx_s.fd[0], (struct sockaddr *) &srv_addr, sizeof(srv_addr));
        if (ret < 0)
        {
                LOGGER_ERR("Failed to bind server socket [fd=%d]", ctx_s.fd[0]);
                server_us_nb_remove_fd(ctx_s.fd[0]);
                return;
        }

        /* Listen on the socket with 5 pending connections maximum */
        ret = listen(ctx_s.fd[0], 5);
        if (ret != 0)
        {
                LOGGER_ERR("Failed to listen on server socket [fd=%d]", ctx_s.fd[0]);
                server_us_nb_remove_fd(ctx_s.fd[0]);
                return;
        }

        LOGGER_DEBUG("Server socket ready to accept clients [fd=%d ; callback=%p]", ctx_s.fd[0], &server_us_nb_handler);
}


/**
 * @brief Stop the block
 */
static void server_us_nb_stop()
{
        int i;

        LOGGER_INFO("Stop block server_us_nb");

        /* Close every file descriptors */
        for (i = 0 ; i < SOCKET_FD_MAX ; i++)
        {
                server_us_nb_remove(i);
        }

        /* Initialize stats */
        server_us_nb_count = 0;
        server_us_nb_bytes = 0;

        /* Remove UNIX socket */
        unlink(SOCKET_NAME);

}


static void server_us_nb_ctrl(enum bk_cmd cmd, void *arg)
{
        (void) arg;

        switch (cmd)
        {
        case BK_INIT:
        {
                server_us_nb_init();
                break;
        }
        case BK_START:
        {
                server_us_nb_start();
                break;
        }
        case BK_STOP:
        {
                server_us_nb_stop();
                break;
        }
        default:
        {
                LOGGER_ERR("Unknown command called [bk_cmd=%d]", cmd);
                return;
        }
        }
}


/**
 * @brief Dump statistics of the block in a string
 *
 * @param buf : String to dump statistics
 * @param len : Size of the string
 *
 * @return Actual size written
 */
static size_t server_us_nb_get_stats(char *buf, size_t len)
{
        int    ret;
        size_t count;

        LOGGER_DEBUG("Get statistics for block server_us_nb [buf=%p ; len=%lu]", buf, len);

        ret = snprintf(buf, len, "%d", ctx_s.fd_count);
        if (ret < 0)
        {
                LOGGER_ERR("snprintf failed to write statistics into buffer [buf=%p ; len=%lu]", buf, len);
                return 0;
        }
        else
        {
                count = (size_t) ret;
        }

        if (count > len)
        {
                return len;
        }
        else
        {
                return count;
        }
}


/* Declare the interface for this block */
struct bk_if server_us_nb_entry =
{
        .ctx = NULL,

        .stats = server_us_nb_get_stats,

        .rx   = NULL,
        .tx   = NULL,
        .ctrl = server_us_nb_ctrl,
};


