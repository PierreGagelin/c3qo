

#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>

#include "c3qo/block.h"


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
        fprintf(stdout, "Block inotify_sb is being initialized\n");

        inotify_sb_ctx_init();

        ctx->fd = inotify_init1(0);
        if (ctx->fd == -1)
        {
                fprintf(stderr, "Failed to init inotify\n");
                exit(EXIT_FAILURE);
        }

        /* Add a pathname to watch */
        if (inotify_add_watch(ctx->fd, "/tmp/toto/", IN_CLOSE_WRITE) == -1)
        {
                fprintf(stderr, "Failed to watch pathname\n");
                exit(EXIT_FAILURE);
        }
}


static void inotify_sb_start()
{
        fd_set         rfds;
        struct timeval tv;
        int            retval;

        fprintf(stdout, "Block inotify_sb is being started\n");

        FD_ZERO(&rfds);
        FD_SET(ctx->fd, &rfds);

        tv.tv_sec  = 50;
        tv.tv_usec = 0;

        retval = select(ctx->fd + 1, &rfds, NULL, NULL, &tv);

        if (retval == -1)
        {
                fprintf(stderr, "select() failed\n");
                exit(EXIT_FAILURE);
        }
        else if (retval)
        {
                printf("Data is available now.\n");
        }
        else
        {
                fprintf(stdout, "No data within fifty seconds.\n");
        }
}


static void inotify_sb_ctrl(enum block_event event, void *arg)
{
        (void) arg;

        switch (event)
        {
        case BLOCK_INIT:
        {
                inotify_sb_init();
                break;
        }
        case BLOCK_START:
        {
                inotify_sb_start();
                break;
        }
        case BLOCK_STOP:
        {
                inotify_sb_ctx_clean();
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


struct block_if inotify_sb_entry =
{
        .rx   = NULL,
        .tx   = NULL,
        .ctrl = inotify_sb_ctrl,
};


