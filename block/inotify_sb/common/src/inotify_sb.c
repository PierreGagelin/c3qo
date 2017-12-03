


#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>

#include "../../../block.h"

void inotify_sb_ctx_init();
void inotify_sb_ctx_clean();
void inotify_sb_ctrl(enum block_event event, void *arg);

struct block_if inotify_sb_entry =
{
        NULL,
        NULL,
        NULL,
        NULL,

        NULL,
        inotify_sb_ctx_init,
        inotify_sb_ctx_clean,

        NULL,
        NULL,
        NULL,
        NULL,

        NULL,
        NULL,
        inotify_sb_ctrl,
};


struct inotify_sb_ctx
{
        int fd; // file descriptor of the inotify
};

void inotify_sb_ctx_init()
{
       inotify_sb_entry.ctx = malloc(sizeof(struct inotify_sb_ctx));
}


void inotify_sb_ctx_clean()
{
        free(inotify_sb_entry.ctx);
}


/**
 * @brief Create a inotify descriptor
 */
void inotify_sb_init()
{
        struct inotify_sb_ctx *ctx;

        ctx = (struct inotify_sb_ctx *) inotify_sb_entry.ctx;

        fprintf(stdout, "Block inotify_sb is being initilized\n");

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



void inotify_sb_start()
{
        struct inotify_sb_ctx *ctx;
        fd_set                rfds;
        struct timeval        tv;
        int                   retval;

        ctx = (struct inotify_sb_ctx *) inotify_sb_entry.ctx;

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



void inotify_sb_ctrl(enum block_event event, void *arg)
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
        default:
        {
                fprintf(stderr, "Unknown event called\n");
                exit(EXIT_FAILURE);
                break;
        }
        }
}



