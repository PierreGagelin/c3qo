

// Project headers
#include "c3qo/manager.hpp"

bk_hello::bk_hello(struct manager *mgr) : block(mgr) {}

void bk_hello::init_()
{
    struct hello_ctx *ctx;

    ctx = new struct hello_ctx;

    // Default values :
    //   - register the block ID
    //   - no name
    //   - no packet processed
    ctx->name[0] = '\0';
    ctx->count = 0;

    ctx_ = ctx;
}

void bk_hello::conf_(char *conf)
{
    struct hello_ctx *ctx;
    size_t len;

    // Verify input
    if ((ctx_ == nullptr) || (conf == nullptr))
    {
        LOGGER_ERR("Failed to configure block: nullptr context or conf");
        return;
    }
    ctx = static_cast<struct hello_ctx *>(ctx_);
    len = strnlen(conf, sizeof(ctx->name));
    if (len == sizeof(ctx->name))
    {
        LOGGER_ERR("Failed to configure block: name too long [bk_id=%d ; name=%s]", id_, conf);
        return;
    }

    LOGGER_INFO("Configure block name [bk_id=%d ; name=%s]", id_, conf);

    // Write name given by configuration
    memcpy(ctx->name, conf, len + 1);
}

void bk_hello::start_()
{
    LOGGER_DEBUG("Hello world");
}

void bk_hello::stop_()
{
    if (ctx_ == nullptr)
    {
        LOGGER_ERR("Failed to stop block: nullptr context");
        return;
    }

    delete static_cast<struct hello_ctx *>(ctx_);

    ctx_ = nullptr;
}

int bk_hello::rx_(void *vdata)
{
    struct hello_ctx *ctx;
    int ret;

    if (ctx_ == nullptr)
    {
        LOGGER_ERR("Failed to process RX data: nullptr context");
        return 0;
    }
    ctx = static_cast<struct hello_ctx *>(ctx_);

    LOGGER_DEBUG("Process RX data [bk_id=%d ; data=%p]", id_, vdata);

    // Get and increment index to return
    ret = ctx->count++ % 8;

    return ret;
}

int bk_hello::tx_(void *vdata)
{
    struct hello_ctx *ctx;
    int ret;

    if (ctx_ == nullptr)
    {
        LOGGER_ERR("Failed to process TX data: nullptr context");
        return 0;
    }
    ctx = static_cast<struct hello_ctx *>(ctx_);

    LOGGER_DEBUG("Process TX data [bk_id=%d ; data=%p]", id_, vdata);

    // Get and increment index to return
    ret = ctx->count++ % 8;

    return ret;
}

int bk_hello::ctrl_(void *vnotif)
{
    struct hello_ctx *ctx;

    if (ctx_ == nullptr)
    {
        LOGGER_ERR("Failed to notify block: nullptr context");
        return 0;
    }
    ctx = static_cast<struct hello_ctx *>(ctx_);

    // Send a message
    process_tx_(ctx->count % 8, vnotif);

    // No forwarding
    return 0;
}

size_t bk_hello::get_stats_(char *buf, size_t len)
{
    int ret;
    size_t written;
    struct hello_ctx *ctx;

    if ((ctx_ == nullptr) || (buf == nullptr) || (len == 0))
    {
        LOGGER_ERR("Failed to get block statistics: nullptr context or nullptr buffer");
        return 0;
    }
    ctx = static_cast<struct hello_ctx *>(ctx_);

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
