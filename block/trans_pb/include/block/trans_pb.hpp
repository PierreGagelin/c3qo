#ifndef TRANS_PB_HPP
#define TRANS_PB_HPP

// Project headers
#include "engine/block.hpp"

struct trans_pb : block
{
    explicit trans_pb(struct manager *mgr);
    virtual ~trans_pb() override final;

    virtual int rx_(void *vdata) override final;

    void parse_command(const uint8_t *data, size_t size);
};

#endif // TRANS_PB_HPP
