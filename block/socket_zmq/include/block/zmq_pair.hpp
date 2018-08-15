#ifndef BLOCK_PUB_SUB_HPP
#define BLOCK_PUB_SUB_HPP

// Project headers
#include "c3qo/block.hpp"

#define XSTR(X) STR(X)
#define STR(X) #X

// Size reserved for an fully specified address (#balanceTonPort)
// e.g.: "tcp://xxx.xxx.xxx.xxx:nnnnn" is 28 characters long so ADDR_SIZE should be at least 28
#define ADDR_SIZE 32
#define ADDR_FORMAT "%" XSTR(ADDR_SIZE) "s"

// Needle to look for in configuration
#define NEEDLE_TYPE "type=" // either "server" or "client"
#define NEEDLE_ADDR "addr=" // fully specified address

// TODO: only used for protobuf stats POC, need to be updated
//       look for hello, zmq_pair and trans_pb blocks
struct zmq_pair_ctx
{
    int bk_id;
};

struct bk_zmq_pair : block
{
    // Context
    void *zmq_ctx_;
    void *zmq_sock_;

    // Configuration
    bool client_;      // either client or server
    std::string addr_; // address to connect or bind

    // Statistics
    unsigned long rx_pkt_;
    unsigned long tx_pkt_;

    bk_zmq_pair(struct manager *mgr);
    virtual ~bk_zmq_pair() override final;

    virtual void conf_(char *conf) override final;
    virtual void start_() override final;
    virtual void stop_() override final;
    virtual int tx_(void *vdata) override final;
};

#endif // BLOCK_PUB_SUB_HPP
