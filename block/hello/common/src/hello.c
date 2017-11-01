


#include <stdio.h>
#include <stdlib.h>

#include "../../../block.h"



void hello_init()
{
        puts("Block hello is being initilized");
}



void hello_start()
{
        puts("Hello world");
}



void hello_ctrl(void * b_conf, void * b_stats, void * b_ctx,
                enum block_event event, void * arg)
{
        (void) b_conf;
        (void) b_ctx;
        (void) b_stats;
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



struct block_if hello_entry =
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
        hello_ctrl,
};

