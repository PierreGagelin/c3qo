/**
 * @brief Implement a AF_UNIX NON-BLOCKING client socket
 *          - NON-BLOCKING : return error code instead of blocking
 *          - AF_UNIX      : socket domain and SOCK_STREAM type
 *
 * @note us_asnb stand for unix stream async non-block
 */

/* WARN: non-POSIX */
#define _GNU_SOURCE

#include <sys/un.h>     /* sockaddr_un */
#include <sys/socket.h> /* socket */

#include "c3qo/block.h"
#include "c3qo/logger.h"
#include "c3qo/signal.h"
#include "c3qo/socket.h"


struct client_us_nb_ctx
{
        int fd; /**< File descriptor of the socket */
};
struct client_us_nb_ctx ctx;


/**
 * @brief Callback for SIGIO
 *
 * NOTE: it is dangerous to make syscalls here
 */
static void client_us_nb_handler(int sig, siginfo_t *info, void *context)
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
static void client_us_nb_init()
{
        struct sockaddr_un clt_addr;
        const char         *buff;
        int                ret;

        LOGGER_INFO("Block client_us_nb is being initialized");

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
        c3qo_register_fd_handler(SIGIO, client_us_nb_handler);
        c3qo_socket_set_asnb(ctx.fd);

        memset(&clt_addr, 0, sizeof(clt_addr));
        clt_addr.sun_family = AF_UNIX;
        strcpy(clt_addr.sun_path, "/tmp/server_us_nb");

        ret = connect(ctx.fd, (struct sockaddr *) &clt_addr, sizeof(clt_addr));
        if (ret == -1)
        {
                LOGGER_ERR("Failed to connect socket");
                return;
        }

        buff = "client_us_nb world!\n";
        
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
static void client_us_nb_start()
{
        LOGGER_DEBUG("Not implemented yet");
}


static void client_us_nb_ctrl(enum bk_cmd cmd, void *arg)
{
        (void) arg;

        switch (cmd)
        {
        case BK_INIT:
        {
                client_us_nb_init();
                break;
        }
        case BK_START:
        {
                client_us_nb_start();
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
struct bk_if client_us_nb_entry =
{
        .ctx = NULL,

        .rx   = NULL,
        .tx   = NULL,
        .ctrl = client_us_nb_ctrl,
};

