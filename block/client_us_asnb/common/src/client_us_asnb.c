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

/* include from the project itself */
#include "../../../block.h"
#include "../../../../engine/common/include/c3qo_signal.h"
#include "../../../../engine/common/include/c3qo_socket.h"

/* include from external libraries */
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>

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
void client_us_asnb_handler(int sig, siginfo_t * info, void * context)
{
        (void) context;
        (void) info;

        fprintf(stdout, "client_us_asnb SIGIO handler called\n");

        if (sig != SIGIO)
        {
                fprintf(stderr, "ERROR: bad signal\n");
                exit(EXIT_FAILURE);
        }
}



/**
 * @brief Initialization function
 */
void client_us_asnb_init()
{
        struct sockaddr_un clt_addr;
        const char *       buff;
        int                ret;

        puts("Block client_us_asnb is being initialized");

        /* context initialization */
        memset(&ctx, -1, sizeof(ctx));

        /* creation of the client socket */
        ctx.fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (ctx.fd == -1)
        {
                fprintf(stderr, "ERROR while opening socket");
                exit(EXIT_FAILURE);
        }

        /* set the socket to be ASNB and register SIGIO handler */
        c3qo_register_fd_handler(SIGIO, client_us_asnb_handler);
        c3qo_socket_set_asnb(ctx.fd);

        memset(&clt_addr, 0, sizeof(clt_addr));
        clt_addr.sun_family = AF_UNIX;
        strcpy(clt_addr.sun_path, "/tmp/server_us_asnb");

        ret = connect(ctx.fd, (struct sockaddr *) &clt_addr,
                        sizeof(clt_addr));
        if (ret == -1)
        {
                fprintf(stdout, "ERROR: couldn't connect to server\n");
                exit(EXIT_FAILURE);
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
void client_us_asnb_start()
{
        fprintf(stdout, "Not implemented yet\n");
}

void client_us_asnb_ctrl(enum block_event event, void * arg)
{
        (void) arg;

        switch (event)
        {
        case BLOCK_INIT:
        {
                client_us_asnb_init();
                break;
        }
        case BLOCK_START:
        {
                client_us_asnb_start();
                break;
        }
        default:
        {
                fprintf(stderr, "Unknown event called\n");
                exit(EXIT_FAILURE);
                break;
        }
        }
}



/* Declare the interface for this block */
struct block_if client_us_asnb_entry =
{
        NULL,
        NULL,
        NULL,
        NULL,

        NULL,
        NULL,
        NULL,

        NULL,
        NULL,
        NULL,
        NULL,

        NULL,
        NULL,
        client_us_asnb_ctrl,
};

