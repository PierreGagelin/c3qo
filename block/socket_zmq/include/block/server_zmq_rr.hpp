#ifndef BLOCK_SERVER_ZMQ_RR_HPP
#define BLOCK_SERVER_ZMQ_RR_HPP

// Project headers
#include "c3qo/block.hpp"

//
// @struct server_zmq_rr_ctx
//
// @brief Internal structure to store the context
//
struct server_zmq_rr_ctx
{
    int bk_id;
    
    void *zmq_ctx;
    void *zmq_socket_pub;
    void *zmq_socket_sub;

    unsigned long rx_pkt_count;
    unsigned long tx_pkt_count;
};

extern struct bk_if server_zmq_rr_if;

#endif // BLOCK_SERVER_ZMQ_RR_HPP
