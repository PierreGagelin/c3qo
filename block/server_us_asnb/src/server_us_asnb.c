/**
 * @brief Implement an ASYNCHRONOUS and NON-BLOCKING socket
 *          - ASYNCHRONOUS : signal SIGIO wake the file descriptor up
 *          - NON-BLOCKING : throw errors instead of blocking
 *          - AF_UNIX domain and SOCK_STREAM type
 *
 * @note us_asnb stand for unix stream async non-block
 */


#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "c3qo/block.h"
#include "c3qo/logger.h"
#include "c3qo/signal.h"
#include "c3qo/socket.h"


#define SOCKET_FD_MAX    64 /* maximum number of file descriptors */
#define SOCKET_READ_SIZE 256


/* context of the block */
struct server_us_asnb_ctx
{
        int fd[SOCKET_FD_MAX]; /* file descriptors of the socket */
        int fd_count;          /* number of fd in use */
};
struct server_us_asnb_ctx ctx;


/* statistics */
unsigned int server_us_asnb_count;
ssize_t      server_us_asnb_bytes;


/**
 * @brief Flush a file descriptor
 */
static void server_us_asnb_flush_fd(int fd)
{
        ssize_t ret;
        char    buff[SOCKET_READ_SIZE];

        do
        {
                memset(buff, 0, sizeof(buff));
                ret = c3qo_socket_read_nb(fd, buff, sizeof(buff));
                if (ret != -1)
                {
                        server_us_asnb_count += 1;
                        server_us_asnb_bytes += ret;
                }
        } while (ret != -1);
}


/**
 * @brief Callback for SIGIO
 *
 * NOTE: it is dangerous to make syscalls here
 */
static void server_us_asnb_handler(int sig, siginfo_t *info, void *context)
{
        (void) context;

        if (sig != SIGIO)
        {
                LOGGER_ERR("bad signal");
                exit(EXIT_FAILURE);
        }

        if (info->si_fd == ctx.fd[0])
        {
                struct sockaddr_un client;
                socklen_t          size;
                int                ret;

                /* new connection has arrived */
                size = sizeof(client);
                ret = accept(ctx.fd[0], (struct sockaddr *) &client, &size);
                if (ret == -1)
                {
                        LOGGER_ERR("Failed to accept new client");
                        exit(EXIT_FAILURE);
                }

                /* keep the new file descriptor */
                ctx.fd[1] = ret;
                c3qo_socket_set_asnb(ctx.fd[1]);
        }
        else if (info->si_fd == ctx.fd[1])
        {
                server_us_asnb_flush_fd(ctx.fd[1]);

                LOGGER_DEBUG("dump stats : count=%u, bytes=%ld",
                                server_us_asnb_count,
                                server_us_asnb_bytes);
        }
        else
        {
                LOGGER_ERR("Unknown descriptor called");
        }
}


/**
 * @brief Initialization function
 */
static void server_us_asnb_init()
{
        struct sockaddr_un srv_addr;
        int                ret;

        LOGGER_INFO("Block server_us_asnb is being initialized");

        /* context initialization */
        memset(&ctx, -1, sizeof(ctx));
        ctx.fd_count = 0;

        /* creation of the server socket */
        ctx.fd[0] = socket(AF_UNIX, SOCK_STREAM, 0);
        if (ctx.fd[0] == -1)
        {
                LOGGER_ERR("Failed to open socket");
                exit(EXIT_FAILURE);
        }

        /* set the socket to be ASNB and its handler */
        c3qo_register_fd_handler(SIGIO, server_us_asnb_handler);
        c3qo_socket_set_asnb(ctx.fd[0]);

        memset(&srv_addr, 0, sizeof(srv_addr));
        srv_addr.sun_family = AF_UNIX;
        strcpy(srv_addr.sun_path, "/tmp/server_us_asnb");

        /* close an eventual old socket and bind the new one */
        unlink("/tmp/server_us_asnb");
        ret = bind(ctx.fd[0],
                        (struct sockaddr *) &srv_addr,
                        sizeof(srv_addr));
        if (ret < 0)
        {
                LOGGER_ERR("Failed to bind socket");
                exit(EXIT_FAILURE);
        }

        ret = listen(ctx.fd[0], SOCKET_FD_MAX -1);
        if (ret == -1)
        {
                LOGGER_ERR("Failed to listen on socket");
                exit(EXIT_FAILURE);
        }
}


/**
 * @brief Initialization function
 */
static void server_us_asnb_start()
{
        LOGGER_DEBUG("Not implemented yet");
}


static void server_us_asnb_ctrl(enum bk_cmd cmd, void *arg)
{
        (void) arg;

        switch (cmd)
        {
        case BK_INIT:
        {
                server_us_asnb_init();
                break;
        }
        case BK_START:
        {
                server_us_asnb_start();
                break;
        }
        default:
        {
                LOGGER_ERR("Unknown cmd called");
                exit(EXIT_FAILURE);
                break;
        }
        }
}


/* Declare the interface for this block */
struct bk_if server_us_asnb_entry =
{
        .ctx = NULL,

        .rx   = NULL,
        .tx   = NULL,
        .ctrl = server_us_asnb_ctrl,
};

