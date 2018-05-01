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
// @struct pub_sub_ctx
//
// @brief Internal structure to store the context
//
struct pub_sub_ctx
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

extern struct bk_if pub_sub_if;

#endif // BLOCK_PUB_SUB_HPP
