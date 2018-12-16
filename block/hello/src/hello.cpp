

#define LOGGER_TAG "[block.hello]"

// Project headers
#include "block/hello.hpp"

//
// Implementation of the block interface
//

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

    LOGGER_INFO("Configured block name [bk_id=%d ; name=%s]", id_, name_.c_str());
}

void hello::start_()
{
    LOGGER_DEBUG("Hello world");
}

void hello::stop_()
{
    LOGGER_DEBUG("Goodbye world");
}

int hello::data_(void *)
{
    int ret;

    LOGGER_DEBUG("Process data [bk_id=%d]", id_);

    // Get and increment index to return
    ret = 1;
    ret += count_++ % 8;

    return ret;
}

void hello::ctrl_(void *)
{
    LOGGER_DEBUG("Process notification [bk_id=%d]", id_);

    // Increment index to return
    ++count_;
}

//
// Implementation of the factory interface
//

struct block *hello_factory::constructor(struct manager *mgr)
{
    return new struct hello(mgr);
}

void hello_factory::destructor(struct block *bk)
{
    delete static_cast<struct hello *>(bk);
}
