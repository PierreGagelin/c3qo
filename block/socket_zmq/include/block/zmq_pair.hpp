#ifndef BLOCK_PUB_SUB_HPP
#define BLOCK_PUB_SUB_HPP

// Project headers
#include "c3qo/block.hpp"

// TODO: only used for protobuf stats POC, need to be updated
//       look for hello, zmq_pair and trans_pb blocks
struct zmq_pair_ctx
{
    int bk_id;
};

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

    virtual void conf_(char *conf) override final;
    virtual void start_() override final;
    virtual void stop_() override final;
    virtual int tx_(void *vdata) override final;

    virtual void on_fd_(struct file_desc &fd) override final;
};

#endif // BLOCK_PUB_SUB_HPP
