
#include <stdio.h>
#include <stdlib.h>

#include "../../../block.h"



void goodbye_init()
{
        puts("Block goodbye is being initilized");
}



void goodbye_start()
{
        puts("Goodbye world");
}



void goodbye_ctrl(enum block_event event, void *arg)
{
        (void) arg;

        switch (event)
        {
        case BLOCK_INIT:
        {
                goodbye_init();
                break;
        }
        case BLOCK_START:
        {
                goodbye_start();
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



struct block_if goodbye_entry =
{
        .rx   = NULL,
        .tx   = NULL,
        .ctrl = goodbye_ctrl,
};

