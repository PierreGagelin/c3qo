

#include <stdio.h>
#include <stdlib.h>

#include "c3qo/block.hpp"
#include "c3qo/logger.hpp"


static inline void hello_init()
{
        LOGGER_INFO("Block hello is being initialized");
}


static inline void hello_start()
{
        LOGGER_INFO("Block hello is being started");

        LOGGER_DEBUG("Hello world");
}


static inline void hello_stop()
{
        LOGGER_INFO("Block hello is being stopped");
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
        case BK_STOP:
        {
                hello_stop();
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

        .stats = NULL,

        .rx   = NULL,
        .tx   = NULL,
        .ctrl = hello_ctrl,
};


