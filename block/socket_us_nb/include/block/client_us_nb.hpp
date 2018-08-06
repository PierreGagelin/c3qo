#ifndef BLOCK_CLIENT_US_NB_HPP
#define BLOCK_CLIENT_US_NB_HPP

// Project headers
#include "c3qo/block.hpp"

struct bk_client_us_nb : block
{
    // Configuration
    int port_;  // Binding port

    // Context
    int fd_;         // Socket file descriptor
    bool connected_; // True if socket is connected

    // Statistics
    size_t rx_pkt_; // RX: Number of packets read
    size_t tx_pkt_; // TX: Number of packets sent

    bk_client_us_nb(struct manager *mgr);

    void connect_();
    void clean_();

    virtual void start_() override final;
    virtual void stop_() override final;
    virtual int tx_(void *vdata) override final;
};

#endif // BLOCK_CLIENT_US_NB_HPP
