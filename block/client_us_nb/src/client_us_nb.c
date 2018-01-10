/**
 * @brief Implement an ASYNCHRONOUS and NON-BLOCKING client socket
 *          - ASYNCHRONOUS : signal SIGIO wake the file descriptor up
 *          - NON-BLOCKING : throw errors instead of blocking
 *          - AF_UNIX domain and SOCK_STREAM type
 *
 * @note us_asnb stand for unix stream async non-block
 */

/* WARN: non-POSIX
 * SIGIO management could be replaced by aio_sigevent */
#define _GNU_SOURCE

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>

#include "c3qo/block.h"
#include "c3qo/logger.h"
#include "c3qo/signal.h"
#include "c3qo/socket.h"


struct client_us_asnb_ctx
{
        int fd; /**< File descriptor of the socket */
};
struct client_us_asnb_ctx ctx;


/**
 * @brief Callback for SIGIO
 *
 * NOTE: it is dangerous to make syscalls here
 */
static void client_us_asnb_handler(int sig, siginfo_t *info, void *context)
{
        (void) context;
        (void) info;

        if (sig != SIGIO)
        {
                LOGGER_ERR("bad signal");
        }
}


/**
 * @brief Initialization function
 */
static void client_us_asnb_init()
{
        struct sockaddr_un clt_addr;
        const char         *buff;
        int                ret;

        LOGGER_INFO("Block client_us_asnb is being initialized");

        /* context initialization */
        memset(&ctx, -1, sizeof(ctx));

        /* creation of the client socket */
        ctx.fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (ctx.fd == -1)
        {
                LOGGER_ERR("Failed to open socket");
                return;
        }

        /* set the socket to be ASNB and register SIGIO handler */
        c3qo_register_fd_handler(SIGIO, client_us_asnb_handler);
        c3qo_socket_set_asnb(ctx.fd);

        memset(&clt_addr, 0, sizeof(clt_addr));
        clt_addr.sun_family = AF_UNIX;
        strcpy(clt_addr.sun_path, "/tmp/server_us_asnb");

        ret = connect(ctx.fd, (struct sockaddr *) &clt_addr, sizeof(clt_addr));
        if (ret == -1)
        {
                LOGGER_ERR("Failed to connect socket");
                return;
        }

        buff = "client_us_asnb world!\n";
        
        for (ret=0; ret < 1800; ret++)
        {
                if (c3qo_socket_write_nb(ctx.fd, buff, 256) == -1)
                {
                        ret -= 1;
                        pause();
                }
        }
}


/**
 * @brief Initialization function
 */
static void client_us_asnb_start()
{
        LOGGER_DEBUG("Not implemented yet");
}


static void client_us_asnb_ctrl(enum bk_cmd cmd, void *arg)
{
        (void) arg;

        switch (cmd)
        {
        case BK_INIT:
        {
                client_us_asnb_init();
                break;
        }
        case BK_START:
        {
                client_us_asnb_start();
                break;
        }
        default:
        {
                LOGGER_ERR("Unknown cmd called");
                break;
        }
        }
}


/* Declare the interface for this block */
struct bk_if client_us_asnb_entry =
{
        .ctx = NULL,

        .rx   = NULL,
        .tx   = NULL,
        .ctrl = client_us_asnb_ctrl,
};

