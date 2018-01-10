

#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>

#include "c3qo/block.h"
#include "c3qo/logger.h"


struct inotify_sb_ctx
{
        int fd; // file descriptor of the inotify
};

/* context of the block */
struct inotify_sb_ctx *ctx;

static void inotify_sb_ctx_init()
{
       ctx = malloc(sizeof(struct inotify_sb_ctx));
}


static void inotify_sb_ctx_clean()
{
        free(ctx);
}


/**
 * @brief Create a inotify descriptor
 */
static void inotify_sb_init()
{
        LOGGER_INFO("Block inotify_sb is being initialized");

        inotify_sb_ctx_init();

        ctx->fd = inotify_init1(0);
        if (ctx->fd == -1)
        {
                LOGGER_ERR("Failed to init inotify");
                return;
        }

        /* Add a pathname to watch */
        if (inotify_add_watch(ctx->fd, "/tmp/toto/", IN_CLOSE_WRITE) == -1)
        {
                LOGGER_ERR("Failed to watch pathname");
                return;
        }
}


static void inotify_sb_start()
{
        fd_set         rfds;
        struct timeval tv;
        int            retval;

        LOGGER_INFO("Block inotify_sb is being started");

        FD_ZERO(&rfds);
        FD_SET(ctx->fd, &rfds);

        tv.tv_sec  = 50;
        tv.tv_usec = 0;

        retval = select(ctx->fd + 1, &rfds, NULL, NULL, &tv);

        if (retval == -1)
        {
                LOGGER_ERR("select() failed");
                return;
        }
        else if (retval)
        {
                LOGGER_DEBUG("Data is available now");
        }
        else
        {
                LOGGER_DEBUG("No data within fifty seconds");
        }
}


static void inotify_sb_ctrl(enum bk_cmd cmd, void *arg)
{
        (void) arg;

        switch (cmd)
        {
        case BK_INIT:
        {
                inotify_sb_init();
                break;
        }
        case BK_START:
        {
                inotify_sb_start();
                break;
        }
        case BK_STOP:
        {
                inotify_sb_ctx_clean();
                break;
        }
        default:
        {
                LOGGER_ERR("Unknown cmd called");
                return;
        }
        }
}


struct bk_if inotify_sb_entry =
{
        .ctx = NULL,

        .rx   = NULL,
        .tx   = NULL,
        .ctrl = inotify_sb_ctrl,
};


