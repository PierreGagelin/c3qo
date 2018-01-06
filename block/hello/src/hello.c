
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
        LOGGER_INFO("Block hello is being started");

        LOGGER_DEBUG("Hello world");
}


static void hello_ctrl(enum bk_cmd cmd, void *arg)
{
        (void) arg;

        LOGGER_DEBUG("Block command bk_cmd=%d called with argument arg=%p", cmd, arg);

        switch (cmd)
        {
        case BK_INIT:
        {
                hello_init();
                break;
        }
        case BK_START:
        {
                hello_start();
                break;
        }
        default:
        {
                LOGGER_ERR("Unknown bk_cmd=%d called", cmd);
                break;
        }
        }
}


/**
 * @brief Exported structure of the block
 */
struct bk_if hello_entry =
{
        .ctx = NULL,

        .rx   = NULL,
        .tx   = NULL,
        .ctrl = hello_ctrl,
};

