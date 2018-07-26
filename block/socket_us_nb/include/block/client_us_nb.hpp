#ifndef BLOCK_CLIENT_US_NB_HPP
#define BLOCK_CLIENT_US_NB_HPP

// Project headers
#include "c3qo/block.hpp"

//
// @brief Context of the block
//
struct client_us_nb_ctx
{
    // Configuration
    int bk_id; // Block ID
    int bind;  // Binding ID

    // Context
    int fd;         // Socket file descriptor
    bool connected; // True if socket is connected

    // Statistics
    size_t rx_pkt_count; // RX: Number of packets read
    size_t rx_pkt_bytes; // RX: Total size read
    size_t tx_pkt_count; // TX: Number of packets sent
    size_t tx_pkt_bytes; // TX: Total size sent
};

extern struct bk_if client_us_nb_if;

#endif // BLOCK_CLIENT_US_NB_HPP
