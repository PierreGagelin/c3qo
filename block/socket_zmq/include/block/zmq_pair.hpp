#ifndef ZMQ_PAIR_HPP
#define ZMQ_PAIR_HPP

// Project headers
#include "engine/block.hpp"

struct zmq_pair : block
{
    // Context
    void *zmq_ctx_;
    struct file_desc zmq_sock_;

    // Configuration
    bool client_;      // either client or server
    std::string addr_; // address to connect or bind

    // Statistics
    unsigned long rx_pkt_;
    unsigned long tx_pkt_;

    explicit zmq_pair(struct manager *mgr);
    virtual ~zmq_pair() override final;

    virtual void start_() override final;
    virtual void stop_() override final;

    virtual int data_(void *vdata) override final;

    virtual void on_fd_(struct file_desc &fd) override final;
};

struct zmq_pair_factory : block_factory
{
    virtual struct block *constructor(struct manager *mgr) override final;
    virtual void destructor(struct block *bk) override final;
};

#endif // ZMQ_PAIR_HPP
