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

extern struct bk_if trans_pb_if;

#endif // BLOCK_TRANS_PB_HPP
