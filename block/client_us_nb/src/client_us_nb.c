/**
 * @brief Implement a AF_UNIX NON-BLOCKING client socket
 *          - AF_UNIX      : socket domain and SOCK_STREAM type
 *          - NON-BLOCKING : return error code instead of blocking
 *
 * @note us_asnb stand for unix stream non-block
 */


#include <unistd.h>     /* close */
#include <stdio.h>      /* snprintf */
#include <string.h>     /* memset */
#include <sys/types.h>  /* getsockopt */
#include <sys/un.h>     /* sockaddr_un */
#include <sys/socket.h> /* socket, getsockopt */

#include "c3qo/block.h"      /* bk_cmd, bk_data... */
#include "c3qo/logger.h"     /* LOGGER_INFO, LOGGER_ERR... */
#include "c3qo/manager_fd.h" /* manager_fd_add */
#include "c3qo/socket.h"     /* c3qo_socket_set_nb */


#define SOCKET_NAME "/tmp/server_us_nb"


/**
 * @brief Context of the block
 */
struct client_us_nb_ctx
{
        int fd;
};
struct client_us_nb_ctx ctx_c;


/**
 * @brief Remove a managed file descriptor and close it
 */
static inline void client_us_nb_clean()
{
        manager_fd_remove(ctx_c.fd, true);
        manager_fd_remove(ctx_c.fd, false);
        close(ctx_c.fd);
        ctx_c.fd = -1;
}


/**
 * @brief Callback function when data is received
 */
static void client_us_nb_callback(int fd)
{
        if (fd != ctx_c.fd)
        {
                LOGGER_ERR("Unexpected file descriptor [fd_expected=%d ; fd_received=%d]", ctx_c.fd, fd);
                return;
        }

        LOGGER_DEBUG("Received data on socket. Not implemented yet [fd=%d]", fd);
}


/**
 * @brief Check that the socket is connected
 */
static void client_us_nb_connect_check(int fd)
{
        socklen_t          lon;
        int                optval;

        if (fd != ctx_c.fd)
        {
                LOGGER_ERR("Unexpected file descriptor [fd_expected=%d ; fd_received=%d]", ctx_c.fd, fd);
                return;
        }

        /* Verify connection status */
        lon = sizeof(optval);
        if (getsockopt(ctx_c.fd, SOL_SOCKET, SO_ERROR, (void *)(&optval), &lon) != 0)
        {
                LOGGER_ERR("getsockopt failed on socket [fd=%d]", ctx_c.fd);
                return;
        }

        if (optval != 0)
        {
                LOGGER_ERR("SO_ERROR still not clear on socket [fd=%d]", ctx_c.fd);
                return;
        }

        LOGGER_DEBUG("Connection to server available on socket [fd=%d]", ctx_c.fd);

        manager_fd_remove(ctx_c.fd, false);
        manager_fd_add(ctx_c.fd, &client_us_nb_callback, true);
}


/**
 * @brief Connect to a server
 */
static int client_us_nb_connect(int fd)
{
        struct sockaddr_un clt_addr;
        int                ret;

        memset(&clt_addr, 0, sizeof(clt_addr));
        clt_addr.sun_family = AF_UNIX;
        ret = snprintf(clt_addr.sun_path, sizeof(clt_addr.sun_path), SOCKET_NAME);
        if (ret < 0)
        {
                LOGGER_ERR("Failed snprintf [buf=%p ; size=%lu ; string=%s]", clt_addr.sun_path, sizeof(clt_addr.sun_path), SOCKET_NAME);
                return -1;
        }
        else if (((size_t) ret) > sizeof(clt_addr.sun_path))
        {
                LOGGER_ERR("Failed to write socket name, it's too large [sun_path=%s ; max_size=%lu]", SOCKET_NAME, sizeof(clt_addr.sun_path));
                return -1;
        }

        return c3qo_socket_connect_nb(fd, (struct sockaddr *) &clt_addr, sizeof(clt_addr));
}


/**
 * @brief Initialize the block
 */
static void client_us_nb_init()
{
        LOGGER_INFO("Initialize block client_us_nb");

        /* Initialize context */
        memset(&ctx_c, -1, sizeof(ctx_c));
        ctx_c.fd = -1;
}


/**
 * @brief Start the block
 */
static void client_us_nb_start()
{
        int ret;

        LOGGER_INFO("Start block client_us_nb");

        /* Create the client socket */
        ctx_c.fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (ctx_c.fd == -1)
        {
                LOGGER_ERR("Failed to open client socket [fd=%d]", ctx_c.fd);
                return;
        }

        /* Set the socket to be non-blocking */
        c3qo_socket_set_nb(ctx_c.fd);

        /* Connect the socket to the server */
        ret = client_us_nb_connect(ctx_c.fd);
        if (ret == -1)
        {
                LOGGER_ERR("Failed to connect to server [fd=%d]", ctx_c.fd);
                client_us_nb_clean();
                return;
        }
        else if (ret == 1)
        {
                LOGGER_DEBUG("Connection sent but not acknowledged, will check later [fd=%d]", ctx_c.fd);
                manager_fd_add(ctx_c.fd, &client_us_nb_connect_check, false);
                return;
        }
        else if (ret == 2)
        {
                LOGGER_ERR("Failed to find someone listening, launch timer for reconnection [fd=%d]", ctx_c.fd);


                return;
        }
        else
        {
                /* Success: register the file descriptor with a callback for data reception */
                if (manager_fd_add(ctx_c.fd, &client_us_nb_callback, true) == false)
                {
                        LOGGER_ERR("Failed to register callback on client socket [fd=%d ; callback=%p]", ctx_c.fd, &client_us_nb_callback);
                        client_us_nb_clean();
                        return;
                }
                else
                {
                        LOGGER_DEBUG("Registered callback on client socket [fd=%d ; callback=%p]", ctx_c.fd, &client_us_nb_callback);
                }
        }
}


/**
 * @brief Stop the block
 */
static void client_us_nb_stop()
{
        LOGGER_INFO("Stop block client_us_nb");

        if (ctx_c.fd == -1)
        {
                return;
        }

        client_us_nb_clean();
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
        case BK_STOP:
        {
                client_us_nb_stop();
                break;
        }
        default:
        {
                LOGGER_ERR("Unknown command called [bk_cmd=%d]", cmd);
                break;
        }
        }
}


/* Declare the interface for this block */
struct bk_if client_us_nb_entry =
{
        .ctx = NULL,

        .stats = NULL,

        .rx   = NULL,
        .tx   = NULL,
        .ctrl = client_us_nb_ctrl,
};


