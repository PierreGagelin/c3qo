#ifndef HELLO_HPP
#define HELLO_HPP

// Project headers
#include "engine/block.hpp"

struct hello : block
{
    std::string name_;
    int count_;

    explicit hello(struct manager *mgr);
    virtual ~hello() override final;

    virtual void conf_(char *conf) override final;
    virtual void start_() override final;
    virtual void stop_() override final;

    virtual int rx_(void *vdata) override final;
    virtual int tx_(void *vdata) override final;
    virtual int ctrl_(void *vnotif) override final;
};

struct hello_factory : block_factory
{
    virtual struct block *constructor(struct manager *mgr) override final;
    virtual void destructor(struct block *bk) override final;
};

#endif // HELLO_HPP
