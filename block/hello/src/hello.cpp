

// C++ library headers
#include <cstdlib> // malloc
#include <cstring> // memset, strnlen

// Project headers
#include "c3qo/block.hpp"
#include "utils/logger.hpp"

//
// @struct hello_bind
//
// @brief Internal structure to bind a port to a block. Here it is
//        a basic array, but it's up to the block to have another way
//        to deal with its bindings (unordered_map, list...)
//
struct hello_bind
{
    int id[8]; // List of binds for this block
};

//
// @struct hello_conf
//
// @brief Internal structure to store configuration. These entries should
//        be provided by the conf callback around startup time
//
struct hello_conf
{
    char name[64]; // Name of the block
};

//
// @struct hello_ctx
//
// @brief Internal structure to store the context. To limit arguments in
//        callbacks, it also stores conf and bind structures. It aims
//        to store run-time "context" rather than configuration
//
struct hello_ctx
{
    struct hello_conf conf; // Block configuration structure
    struct hello_bind bind; // Block bind structure

    // Context information
    int count; // Number of packets processed
};

void *hello_init()
{
    struct hello_ctx *ctx;

    ctx = (struct hello_ctx *)malloc(sizeof(*ctx));
    if (ctx == NULL)
    {
        LOGGER_ERR("Failed to initialize block");
        return ctx;
    }

    // Default values :
    //   - bind ID to zero (dropped by engine)
    //   - no name
    //   - no packet processed
    memset(ctx->bind.id, 0, sizeof(ctx->bind.id));
    ctx->conf.name[0] = '\0';
    ctx->count = 0;

    LOGGER_INFO("Initialize block [ctx=%p]", ctx);

    return ctx;
}

void hello_conf(void *vctx, char *conf)
{
    struct hello_ctx *ctx;
    size_t len;

    // Verify input
    if ((vctx == NULL) || (conf == NULL))
    {
        LOGGER_ERR("Failed to configure block: NULL context or conf");
        return;
    }
    ctx = (struct hello_ctx *)vctx;
    len = strnlen(conf, sizeof(ctx->conf.name));
    if (len == sizeof(ctx->conf.name))
    {
        LOGGER_ERR("Failed to configure block: name too long [ctx=%p ; name=%s]", ctx, conf);
        return;
    }

    // Write name given by configuration
    memcpy(ctx->conf.name, conf, len + 1);
}

void hello_bind(void *vctx, int port, int bk_id)
{
    struct hello_ctx *ctx;

    // Verify input
    if ((vctx == NULL) || (port < 0) || (port > 7))
    {
        LOGGER_ERR("Failed to bind block");
        return;
    }
    ctx = (struct hello_ctx *)vctx;

    // Bind a port to a block
    ctx->bind.id[port] = bk_id;

    LOGGER_INFO("Bind block [port=%d ; bk_id=%d ; ctx=%p]", port, bk_id, ctx);
}

void hello_start(void *vctx)
{
    struct hello_ctx *ctx;

    if (vctx == NULL)
    {
        LOGGER_ERR("Failed to start block");
        return;
    }
    ctx = (struct hello_ctx *)vctx;

    LOGGER_INFO("Start block [ctx=%p]", ctx);

    LOGGER_DEBUG("Hello world");
}

void hello_stop(void *vctx)
{
    struct hello_ctx *ctx;

    if (vctx == NULL)
    {
        LOGGER_ERR("Failed to stop block");
        return;
    }
    ctx = (struct hello_ctx *)vctx;

    free(ctx);

    LOGGER_INFO("Stop block [ctx=%p]", ctx);
}

int hello_rx(void *vctx, void *vdata)
{
    struct hello_ctx *ctx;
    int ret;

    if (vctx == NULL)
    {
        LOGGER_ERR("Failed to stop block");
        return 0;
    }
    ctx = (struct hello_ctx *)vctx;

    LOGGER_DEBUG("Block received RX data [data=%p ; ctx=%p]", vdata, ctx);

    // Get bind index to return
    ret = ctx->bind.id[ctx->count % 8];

    // Increase packet count
    ctx->count++;

    return ret;
}

int hello_tx(void *vctx, void *vdata)
{
    struct hello_ctx *ctx;
    int ret;

    if (vctx == NULL)
    {
        LOGGER_ERR("Failed to stop block");
        return 0;
    }
    ctx = (struct hello_ctx *)vctx;

    LOGGER_DEBUG("Block received TX data [data=%p ; ctx=%p]", vdata, ctx);

    // Get bind index to return
    ret = ctx->bind.id[ctx->count % 8];

    // Increase packet count
    ctx->count++;

    return ret;
}

void hello_ctrl(void *ctx, void *vnotif)
{
    if (ctx != NULL)
    {
        LOGGER_ERR("Failed to notify block hello, he should have a NULL context [ctx=%p]", ctx);
        return;
    }

    LOGGER_DEBUG("Block received notification [notif=%p ; ctx=%p]", vnotif, ctx);
}

//
// @brief Exported structure of the block
//
struct bk_if hello_entry = {
    .init = hello_init,
    .conf = hello_conf,
    .bind = hello_bind,
    .start = hello_start,
    .stop = hello_stop,

    .get_stats = NULL,

    .rx = hello_rx,
    .tx = hello_tx,
    .ctrl = &hello_ctrl,
};
