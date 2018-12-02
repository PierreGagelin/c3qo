

#define LOGGER_TAG "[block.hello]"

// Project headers
#include "block/hello.hpp"
#include "c3qo/manager.hpp"

hello::hello(struct manager *mgr) : block(mgr), name_("unknown"), count_(0) {}
hello::~hello() {}

void hello::conf_(char *conf)
{
    // Verify input
    if (conf == nullptr)
    {
        LOGGER_ERR("Failed to configure block: nullptr conf");
        return;
    }

    name_ = std::string(conf);

    LOGGER_INFO("Configure block [bk_id=%d ; conf=%s ; name=%s]", id_, conf, name_.c_str());
}

void hello::start_()
{
    LOGGER_DEBUG("Hello world");
}

void hello::stop_()
{
    LOGGER_DEBUG("Goodbye world");
}

int hello::rx_(void *vdata)
{
    int ret;

    LOGGER_DEBUG("Process RX data [bk_id=%d ; data=%p]", id_, vdata);

    // Get and increment index to return
    ret = count_++ % 8;

    return ret;
}

int hello::tx_(void *vdata)
{
    int ret;

    LOGGER_DEBUG("Process TX data [bk_id=%d ; data=%p]", id_, vdata);

    // Get and increment index to return
    ret = count_++ % 8;

    return ret;
}

int hello::ctrl_(void *vnotif)
{
    // Send a message
    process_tx_(count_ % 8, vnotif);

    // No forwarding
    return 0;
}

size_t hello::get_stats_(char *buf, size_t len)
{
    int ret;
    size_t written;

    if ((buf == nullptr) || (len == 0))
    {
        LOGGER_ERR("Failed to get block statistics: nullptr context or nullptr buffer");
        return 0;
    }

    LOGGER_DEBUG("Get block statistics [bk_id=%d ; len=%zu]", id_, len);

    written = 0u;
    ret = snprintf(buf, len, "%d", count_);
    if (ret < 0)
    {
        LOGGER_ERR("Failed snprintf [bk_id=%d ; len=%zu]", id_, len);
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

BLOCK_REGISTER(hello);
