

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
    int bk_id;            // Block ID
    enum bk_type bk_type; // Block type
    char name[64];        // Block name
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

void *hello_init(int bk_id)
{
    struct hello_ctx *ctx;

    ctx = (struct hello_ctx *)malloc(sizeof(*ctx));
    if (ctx == NULL)
    {
        LOGGER_ERR("Failed to initialize block: could not reserve memory for the context");
        return ctx;
    }

    // Default values :
    //   - bind ID to zero (dropped by engine)
    //   - register the block ID
    //   - no name
    //   - no packet processed
    memset(ctx->bind.id, 0, sizeof(ctx->bind.id));
    ctx->conf.bk_id = bk_id;
    ctx->conf.name[0] = '\0';
    ctx->conf.bk_type = TYPE_HELLO;
    ctx->count = 0;

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
        LOGGER_ERR("Failed to configure block: name too long [bk_id=%d ; name=%s]", ctx->conf.bk_id, conf);
        return;
    }

    LOGGER_INFO("Configure block name [bk_id=%d ; name=%s]", ctx->conf.bk_id, conf);

    // Write name given by configuration
    memcpy(ctx->conf.name, conf, len + 1);
}

void hello_bind(void *vctx, int port, int bk_id)
{
    struct hello_ctx *ctx;

    // Verify input
    if ((vctx == NULL) || (port < 0) || (port > 7))
    {
        LOGGER_ERR("Failed to bind block: NULL context or port not in range [port=%d ; range=[0,7]]", port);
        return;
    }
    ctx = (struct hello_ctx *)vctx;

    // Bind a port to a block
    ctx->bind.id[port] = bk_id;
}

void hello_start(void *vctx)
{
    struct hello_ctx *ctx;

    // Verify input
    if (vctx == NULL)
    {
        LOGGER_ERR("Failed to start block: NULL context");
        return;
    }
    ctx = (struct hello_ctx *)vctx;

    LOGGER_DEBUG("Hello world");
}

void hello_stop(void *vctx)
{
    struct hello_ctx *ctx;

    if (vctx == NULL)
    {
        LOGGER_ERR("Failed to stop block: NULL context");
        return;
    }
    ctx = (struct hello_ctx *)vctx;

    free(ctx);
}

int hello_rx(void *vctx, void *vdata)
{
    struct hello_ctx *ctx;
    int ret;

    if (vctx == NULL)
    {
        LOGGER_ERR("Failed to process RX data: NULL context");
        return 0;
    }
    ctx = (struct hello_ctx *)vctx;

    LOGGER_DEBUG("Process RX data [bk_id=%d ; data=%p]", ctx->conf.bk_id, vdata);

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
        LOGGER_ERR("Failed to process TX data: NULL context");
        return 0;
    }
    ctx = (struct hello_ctx *)vctx;

    LOGGER_DEBUG("Process TX data [bk_id=%d ; data=%p]", ctx->conf.bk_id, vdata);

    // Get bind index to return
    ret = ctx->bind.id[ctx->count % 8];

    // Increase packet count
    ctx->count++;

    return ret;
}

void hello_ctrl(void *vctx, void *vnotif)
{
    struct hello_ctx *ctx;

    if (vctx == NULL)
    {
        LOGGER_ERR("Failed to notify block: NULL context");
        return;
    }
    ctx = (struct hello_ctx *)vctx;

    LOGGER_DEBUG("Notify block [bk_id=%d ; notif=%p]", ctx->conf.bk_id, vnotif);
}

//
// @brief Exported structure of the block
//
struct bk_if hello_if = {
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
