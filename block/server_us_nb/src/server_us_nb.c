/**
 * @brief Implement an AF_UNIX NON-BLOCKING socket
 *          - AF_UNIX      : socket domain and SOCK_STREAM type
 *          - NON-BLOCKING : return error code instead of blocking
 *
 * @note us_asnb stand for unix stream non-block
 */


#include <sys/un.h>     /* sockaddr_un */
#include <sys/socket.h> /* socket */

#include "c3qo/block.h"
#include "c3qo/logger.h"
#include "c3qo/manager_fd.h"
#include "c3qo/socket.h"


#define SOCKET_FD_MAX    64 /* maximum number of file descriptors */
#define SOCKET_READ_SIZE 256


/* context of the block */
struct server_us_nb_ctx
{
        int fd[SOCKET_FD_MAX]; /* file descriptors of the socket */
        int fd_count;          /* number of fd in use */
};
struct server_us_nb_ctx ctx;


/* statistics */
static unsigned int server_us_nb_count;
static ssize_t      server_us_nb_bytes;


static inline int server_us_nb_fd_find(int fd)
{
        int i;

        for (i = 0; i < SOCKET_FD_MAX; i++)
        {
                if (ctx.fd[i] != fd)
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
static inline int server_us_nb_fd_add(int fd)
{
        int i;

        c3qo_socket_set_nb(fd);

        if (ctx.fd[ctx.fd_count] != -1)
        {
                /* First easy try */
                i = ctx.fd_count;
        }
        else
        {
                /* Easy try failed, looking for first available */
                i = server_us_nb_fd_find(-1);
        }

        if (i != -1)
        {
                ctx.fd[i] = fd;
                ctx.fd_count++;

                return i;
        }
        else
        {
                LOGGER_ERR("No more room available for new file descriptor");
                return -1;
        }
}


static inline void server_us_nb_fd_remove(int fd)
{
        int i;

        i = server_us_nb_fd_find(fd);

        if (i != -1)
        {
                ctx.fd[i] = -1;
                ctx.fd_count--;
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
                if (ret != -1)
                {
                        server_us_nb_count += 1;
                        server_us_nb_bytes += ret;
                }
        } while (ret != -1);
}


/**
 * @brief Callback when a socket is ready for reading
 *
 * @param fd : file descriptor ready for read
 */
static void server_us_nb_handler(int fd)
{
        if (fd == ctx.fd[0])
        {
                struct sockaddr_un client;
                socklen_t          size;
                int                fd_client;

                /* new connection has arrived */
                size = sizeof(client);
                fd_client = accept(ctx.fd[0], (struct sockaddr *) &client, &size);
                if (fd_client == -1)
                {
                        LOGGER_ERR("Failed to accept new client");
                        return;
                }

                /* keep the new file descriptor */
                if (server_us_nb_fd_add(fd_client) == -1)
                {
                        close(fd_client);
                        return;
                }

                /* register the fd for event */
                if (manager_fd_add(fd_client, &server_us_nb_handler) == false)
                {
                        close(fd_client);
                        server_us_nb_fd_remove(fd_client);
                        return;
                }
        }
        else
        {
                /* Data available from the client */
                server_us_nb_flush_fd(fd);

                LOGGER_DEBUG("dump stats : count=%u, bytes=%ld", server_us_nb_count, server_us_nb_bytes);
        }
}


/**
 * @brief Initialization function
 */
static void server_us_nb_init()
{
        struct sockaddr_un srv_addr;
        int                ret;

        LOGGER_INFO("Block server_us_nb is being initialized");

        /* context initialization */
        memset(&ctx, -1, sizeof(ctx));
        ctx.fd_count = 0;

        /* creation of the server socket */
        ctx.fd[0] = socket(AF_UNIX, SOCK_STREAM, 0);
        ctx.fd_count++;
        if (ctx.fd[0] == -1)
        {
                LOGGER_ERR("Failed to open socket");
                return;
        }

        /* set the socket to be NB */
        c3qo_socket_set_nb(ctx.fd[0]);

        memset(&srv_addr, 0, sizeof(srv_addr));
        srv_addr.sun_family = AF_UNIX;
        strcpy(srv_addr.sun_path, "/tmp/server_us_nb");

        /* close an eventual old socket and bind the new one */
        unlink("/tmp/server_us_nb");
        ret = bind(ctx.fd[0], (struct sockaddr *) &srv_addr, sizeof(srv_addr));
        if (ret < 0)
        {
                LOGGER_ERR("Failed to bind socket");
                return;
        }

        /* register the file descriptor in order to be called when client is ready */
        if (manager_fd_add(ctx.fd[0], &server_us_nb_handler) == false)
        {
                LOGGER_ERR("Failed to register socket fd callback");
        }
        else
        {
                LOGGER_DEBUG("Socket to accept clients is initialized");
        }
}


/**
 * @brief Initialization function
 */
static void server_us_nb_start()
{
        LOGGER_DEBUG("Not implemented yet");
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
        default:
        {
                LOGGER_ERR("Unknown bk_cmd=%d called", cmd);
                return;
        }
        }
}


/* Declare the interface for this block */
struct bk_if server_us_nb_entry =
{
        .ctx = NULL,

        .rx   = NULL,
        .tx   = NULL,
        .ctrl = server_us_nb_ctrl,
};

