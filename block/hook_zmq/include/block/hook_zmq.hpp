#ifndef HOOK_ZMQ_HPP
#define HOOK_ZMQ_HPP

// Project headers
#include "engine/block.hpp"

struct hook_zmq : block
{
    // Context
    void *zmq_ctx_;
    struct file_desc zmq_sock_;

    // Configuration
    bool client_;      // either client or server
    int type_;         // ZMQ socket type
    std::string name_; // identity of this hook
    std::string addr_; // address to connect or bind

    // Statistics
    unsigned long rx_pkt_;
    unsigned long tx_pkt_;

    bool send_(struct buffer &buf);
    void recv_(struct buffer &buf);

    explicit hook_zmq(struct manager *mgr);
    virtual ~hook_zmq() override final;

    virtual void start_() override final;
    virtual void stop_() override final;

    virtual int data_(void *vdata) override final;

    virtual void on_fd_(struct file_desc &fd) override final;
};

struct hook_zmq_factory : block_factory
{
    virtual struct block *constructor(struct manager *mgr) override final;
    virtual void destructor(struct block *bk) override final;
};

#endif // HOOK_ZMQ_HPP
