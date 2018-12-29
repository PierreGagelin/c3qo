#ifndef TRANS_PB_HPP
#define TRANS_PB_HPP

// Project headers
#include "engine/block.hpp"

struct trans_pb : block
{
    explicit trans_pb(struct manager *mgr);
    virtual ~trans_pb() override final;

    virtual bool data_(void *vdata) override final;

    bool proto_command_parse(const uint8_t *data, size_t size);
    void proto_command_reply(bool is_ok);
};

struct trans_pb_factory : block_factory
{
    virtual struct block *constructor(struct manager *mgr) override final;
    virtual void destructor(struct block *bk) override final;
};

#endif // TRANS_PB_HPP
