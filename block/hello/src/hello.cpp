

// Project headers
#include "c3qo/manager.hpp"

// Managers shall be linked
extern struct manager *m;

static void *hello_init(int bk_id)
{
    struct hello_ctx *ctx;

    ctx = new struct hello_ctx;

    // Default values :
    //   - register the block ID
    //   - no name
    //   - no packet processed
    ctx->bk_id = bk_id;
    ctx->name[0] = '\0';
    ctx->count = 0;

    return ctx;
}

static void hello_conf(void *vctx, char *conf)
{
    struct hello_ctx *ctx;
    size_t len;

    // Verify input
    if ((vctx == nullptr) || (conf == nullptr))
    {
        LOGGER_ERR("Failed to configure block: nullptr context or conf");
        return;
    }
    ctx = static_cast<struct hello_ctx *>(vctx);
    len = strnlen(conf, sizeof(ctx->name));
    if (len == sizeof(ctx->name))
    {
        LOGGER_ERR("Failed to configure block: name too long [bk_id=%d ; name=%s]", ctx->bk_id, conf);
        return;
    }

    LOGGER_INFO("Configure block name [bk_id=%d ; name=%s]", ctx->bk_id, conf);

    // Write name given by configuration
    memcpy(ctx->name, conf, len + 1);
}

static void hello_start(void *vctx)
{
    (void)vctx;

    LOGGER_DEBUG("Hello world");
}

static void hello_stop(void *vctx)
{
    struct hello_ctx *ctx;

    if (vctx == nullptr)
    {
        LOGGER_ERR("Failed to stop block: nullptr context");
        return;
    }
    ctx = static_cast<struct hello_ctx *>(vctx);

    delete ctx;
}

static int hello_rx(void *vctx, void *vdata)
{
    struct hello_ctx *ctx;
    int ret;

    if (vctx == nullptr)
    {
        LOGGER_ERR("Failed to process RX data: nullptr context");
        return 0;
    }
    ctx = static_cast<struct hello_ctx *>(vctx);

    LOGGER_DEBUG("Process RX data [bk_id=%d ; data=%p]", ctx->bk_id, vdata);

    // Get and increment index to return
    ret = ctx->count++ % 8;

    return ret;
}

static int hello_tx(void *vctx, void *vdata)
{
    struct hello_ctx *ctx;
    int ret;

    if (vctx == nullptr)
    {
        LOGGER_ERR("Failed to process TX data: nullptr context");
        return 0;
    }
    ctx = static_cast<struct hello_ctx *>(vctx);

    LOGGER_DEBUG("Process TX data [bk_id=%d ; data=%p]", ctx->bk_id, vdata);

    // Get and increment index to return
    ret = ctx->count++ % 8;

    return ret;
}

static int hello_ctrl(void *vctx, void *vnotif)
{
    struct hello_ctx *ctx;

    if (vctx == nullptr)
    {
        LOGGER_ERR("Failed to notify block: nullptr context");
        return 0;
    }
    ctx = static_cast<struct hello_ctx *>(vctx);

    // Send a message
    m->bk.process_tx(ctx->bk_id, ctx->count % 8, vnotif);

    // No forwarding
    return 0;
}

static size_t hello_get_stats(void *vctx, char *buf, size_t len)
{
    int ret;
    size_t written;
    struct hello_ctx *ctx;

    if ((vctx == nullptr) || (buf == nullptr) || (len == 0))
    {
        LOGGER_ERR("Failed to get block statistics: nullptr context or nullptr buffer");
        return 0;
    }
    ctx = static_cast<struct hello_ctx *>(vctx);

    LOGGER_DEBUG("Get block statistics [ctx=%p ; buf=%p ; len=%lu]", ctx, buf, len);

    written = 0u;
    ret = snprintf(buf, len, "%d", ctx->count);
    if (ret < 0)
    {
        LOGGER_ERR("Failed snprintf [ctx=%p ; buf=%p ; len=%lu]", ctx, buf, len);
        return 0;
    }
    else if (static_cast<size_t>(ret) >= len)
    {
        return written;
    }
    else
    {
        written = static_cast<size_t>(ret);
    }

    return written;
}

//
// @brief Exported structure of the block
//
struct bk_if hello_if = {
    .init = hello_init,
    .conf = hello_conf,
    .bind = nullptr,
    .start = hello_start,
    .stop = hello_stop,

    .get_stats = hello_get_stats,

    .rx = hello_rx,
    .tx = hello_tx,
    .ctrl = hello_ctrl,
};
