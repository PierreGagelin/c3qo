

// C++ library headers
#include <cstdio>
#include <cstdlib>

// Project headers
#include "c3qo/block.hpp"
#include "c3qo/logger.hpp"

static inline void goodbye_init()
{
    LOGGER_INFO("Block goodbye is being initilized");
}

static inline void goodbye_start()
{
    LOGGER_INFO("Block goodbye is being started");

    LOGGER_DEBUG("Goodbye world");
}

static inline void goodbye_stop()
{
    LOGGER_INFO("Block goodbye is being stopped");
}

static void goodbye_ctrl(enum bk_cmd cmd, void *arg)
{
    (void)arg;

    switch (cmd)
    {
    case BK_CMD_INIT:
    {
        goodbye_init();
        break;
    }
    case BK_CMD_START:
    {
        goodbye_start();
        break;
    }
    case BK_CMD_STOP:
    {
        goodbye_stop();
        break;
    }
    default:
    {
        LOGGER_ERR("Unknown cmd called");
        break;
    }
    }
}

struct bk_if goodbye_entry =
    {
        .ctx = NULL,

        .stats = NULL,

        .rx = NULL,
        .tx = NULL,
        .ctrl = goodbye_ctrl,
};
