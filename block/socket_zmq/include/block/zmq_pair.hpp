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

//
// @struct zmq_pair_ctx
//
// @brief Internal structure to store the context
//
struct zmq_pair_ctx
{
    // Context
    int bk_id;
    void *zmq_ctx;
    void *zmq_sock;

    // Configuration
    bool client;          // either client or server
    char addr[ADDR_SIZE]; // address to connect or bind

    // Statistics
    unsigned long rx_pkt_count;
    unsigned long tx_pkt_count;
};

struct bk_zmq_pair : block
{
    bk_zmq_pair(struct manager *mgr);

    virtual void init_() override final;
    virtual void conf_(char *conf) override final;
    virtual void start_() override final;
    virtual void stop_() override final;
    virtual int tx_(void *vdata) override final;
};

#endif // BLOCK_PUB_SUB_HPP
