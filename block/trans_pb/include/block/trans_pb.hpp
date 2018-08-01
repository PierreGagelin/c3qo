#ifndef BLOCK_TRANS_PB_HPP
#define BLOCK_TRANS_PB_HPP

// Project headers
#include "c3qo/block.hpp"

enum bk_type
{
    BLOCK_HELLO,
    BLOCK_ZMQ_PAIR,
};

// Structure to fill to notify the block
struct trans_pb_notif
{
    enum bk_type type;
    union {
        struct hello_ctx *hello;
        struct zmq_pair_ctx *zmq_pair;
    } context;
};

struct trans_pb_ctx
{
    int bk_id;
};

#ifdef C3QO_PROTOBUF

struct bk_trans_pb : block
{
    virtual void init_() override final;
    virtual void stop_() override final;
    virtual int ctrl_(void *vnotif) override final;
};

#else

struct bk_trans_pb : block
{
    virtual void init_() override final {}
    virtual void stop_() override final {}
    virtual int ctrl_(void *) override final { return 0; }
};

#endif // C3QO_PROTOBUF

#endif // BLOCK_TRANS_PB_HPP
