

// C++ library headers
#include <cstdlib>

// Project headers
#include "c3qo/block.hpp"
#include "utils/logger.hpp"

void *hello_init()
{
    // Because... why not?
    LOGGER_INFO("Initialize block hello, he has no context to identify him");

    return NULL;
}

void hello_start(void *ctx)
{
    if (ctx != NULL)
    {
        LOGGER_ERR("Failed to start block hello, he should have a NULL context [ctx=%p]", ctx);
        return;
    }

    LOGGER_INFO("Start block [ctx=%p]", ctx);

    LOGGER_DEBUG("Hello world");
}

void hello_stop(void *ctx)
{
    if (ctx != NULL)
    {
        LOGGER_ERR("Failed to stop block hello, he should have a NULL context [ctx=%p]", ctx);
        return;
    }

    LOGGER_INFO("Stop block [ctx=%p]", ctx);
}

void hello_ctrl(void *ctx, void *notif)
{
    if (ctx != NULL)
    {
        LOGGER_ERR("Failed to notify block hello, he should have a NULL context [ctx=%p]", ctx);
        return;
    }

    LOGGER_DEBUG("Block received notification [notif=%p ; ctx=%p]", notif, ctx);
}

//
// @brief Exported structure of the block
//
struct bk_if hello_entry = {
    .init = hello_init,
    .conf = NULL,
    .start = hello_start,
    .stop = hello_stop,

    .get_stats = NULL,

    .rx = NULL,
    .tx = NULL,
    .ctrl = &hello_ctrl,
};
