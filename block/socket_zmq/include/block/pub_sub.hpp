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
#define NEEDLE_SUB "sub_addr="
#define NEEDLE_PUB "pub_addr="

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
    void *zmq_socket_sub;
    void *zmq_socket_pub;

    // Configuration
    char sub_addr[ADDR_SIZE];
    char pub_addr[ADDR_SIZE];

    // Statistics
    unsigned long rx_pkt_count;
    unsigned long tx_pkt_count;
};

extern struct bk_if pub_sub_if;

#endif // BLOCK_PUB_SUB_HPP
