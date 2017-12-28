
#include <stdio.h>
#include <stdlib.h>

#include "c3qo/block.h"
#include "c3qo/logger.h"


static void goodbye_init()
{
        LOGGER_INFO("Block goodbye is being initilized");
}


static void goodbye_start()
{
        LOGGER_INFO("Goodbye world");
}


static void goodbye_ctrl(enum block_cmd cmd, void *arg)
{
        (void) arg;

        switch (cmd)
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
                LOGGER_ERR("Unknown cmd called");
                exit(EXIT_FAILURE);
                break;
        }
        }
}


struct block_if goodbye_entry =
{
        .ctx = NULL,

        .rx   = NULL,
        .tx   = NULL,
        .ctrl = goodbye_ctrl,
};

