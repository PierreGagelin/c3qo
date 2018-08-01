#ifndef BLOCK_HELLO_HPP
#define BLOCK_HELLO_HPP

// Project headers
#include "c3qo/block.hpp"

//
// @struct hello_ctx
//
// @brief Context of the block
//
struct hello_ctx
{
    char name[64]; // Block name
    int bk_id;     // Block identifier
    int count;     // Number of packets processed
};

struct bk_hello : block
{
    virtual void init_() override final;
    virtual void conf_(char *conf) override final;
    virtual void start_() override final;
    virtual void stop_() override final;
    virtual size_t get_stats_(char *buf, size_t len) override final;
    virtual int rx_(void *vdata) override final;
    virtual int tx_(void *vdata) override final;
    virtual int ctrl_(void *vnotif) override final;
};

#endif // BLOCK_HELLO_HPP
