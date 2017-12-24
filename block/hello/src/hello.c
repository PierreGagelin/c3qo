
#include <stdio.h>
#include <stdlib.h>

#include "block/hello.h"


static void hello_init()
{
        puts("Block hello is being initilized");
}


static void hello_start()
{
        puts("Hello world");
}


static void hello_ctrl(enum block_event event, void *arg)
{
        (void) arg;

        switch (event)
        {
        case BLOCK_INIT:
        {
                hello_init();
                break;
        }
        case BLOCK_START:
        {
                hello_start();
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


/**
 * @brief Exported structure of the block
 */
struct block_if hello_entry =
{
        .rx   = NULL,
        .tx   = NULL,
        .ctrl = hello_ctrl,
};

