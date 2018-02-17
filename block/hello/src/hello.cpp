

// C++ library headers
#include <cstring> // memset, strnlen

// Project headers
#include "block/hello.hpp"
#include "c3qo/manager_bk.hpp"
#include "utils/logger.hpp"

// Get an access to the block manager for:
//   - process_notif
//   - process_rx
//   - process_tx
extern class manager_bk m_bk;

void *hello_init(int bk_id)
{
    struct hello_ctx *ctx;

    ctx = (struct hello_ctx *)malloc(sizeof(*ctx));
    if (ctx == NULL)
    {
        LOGGER_ERR("Failed to initialize block: could not reserve memory for the context [bk_id=%d]", bk_id);
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
    (void)vctx;

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

    // Free the context structure
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

int hello_ctrl(void *vctx, void *vnotif)
{
    struct hello_ctx *ctx;

    if (vctx == NULL)
    {
        LOGGER_ERR("Failed to notify block: NULL context");
        return 0;
    }
    ctx = (struct hello_ctx *)vctx;

    // Send a message
    m_bk.process_tx(ctx->bind.id[ctx->count % 8], vnotif);

    // No forwarding
    return 0;
}

size_t hello_get_stats(void *vctx, char *buf, size_t len)
{
    int ret;
    size_t written;
    struct hello_ctx *ctx;

    if ((vctx == NULL) || (buf == NULL) || (len == 0))
    {
        LOGGER_ERR("Failed to get block statistics: NULL context or NULL buffer");
        return 0;
    }
    ctx = (struct hello_ctx *)vctx;

    LOGGER_DEBUG("Get block statistics [ctx=%p ; buf=%p ; len=%lu]", ctx, buf, len);

    written = 0;
    ret = snprintf(buf, len, "%d", ctx->count);
    if (ret < 0)
    {
        LOGGER_ERR("Failed snprintf [ctx=%p ; buf=%p ; len=%lu]", ctx, buf, len);
        return 0;
    }
    else if ((size_t)ret >= len)
    {
        return written;
    }
    else
    {
        written = (size_t)ret;
    }

    return written;
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

    .get_stats = hello_get_stats,

    .rx = hello_rx,
    .tx = hello_tx,
    .ctrl = &hello_ctrl,
};
