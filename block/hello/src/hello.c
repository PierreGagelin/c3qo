
#include <stdio.h>
#include <stdlib.h>

#include "c3qo/block.h"
#include "c3qo/logger.h"


static void hello_init()
{
        LOGGER_INFO("Block hello is being initialized");
}


static void hello_start()
{
        LOGGER_INFO("Hello world");
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
                LOGGER_ERR("Unknown event called\n");
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

