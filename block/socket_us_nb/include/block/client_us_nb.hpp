#ifndef BLOCK_CLIENT_US_NB_HPP
#define BLOCK_CLIENT_US_NB_HPP

// Project headers
#include "c3qo/block.hpp"

struct client_us_nb : block
{
    // Configuration
    int port_;  // Binding port

    // Context
    struct file_desc fd_; // Socket file descriptor
    bool connected_;      // True if socket is connected

    // Statistics
    size_t rx_pkt_; // RX: Number of packets read
    size_t tx_pkt_; // TX: Number of packets sent

    client_us_nb(struct manager *mgr);
    virtual ~client_us_nb() override final;

    void connect_();
    void clean_();

    virtual void start_() override final;
    virtual void stop_() override final;
    virtual int tx_(void *vdata) override final;

    virtual void on_timer_(struct timer &tm) override final;
    virtual void on_fd_(struct file_desc &fd) override final;
};

#endif // BLOCK_CLIENT_US_NB_HPP
