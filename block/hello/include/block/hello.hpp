#ifndef BLOCK_HELLO_HPP
#define BLOCK_HELLO_HPP

// Project headers
#include "c3qo/block.hpp"

// TODO: only used for protobuf stats POC, need to be updated
//       look for hello, zmq_pair and trans_pb blocks
struct hello_ctx
{
    int bk_id;
};

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

#endif // BLOCK_HELLO_HPP
