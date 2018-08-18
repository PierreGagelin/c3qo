#ifndef BLOCK_SERVER_US_NB_HPP
#define BLOCK_SERVER_US_NB_HPP

// Project headers
#include "c3qo/block.hpp"

struct bk_server_us_nb : block
{
    // File descriptors in use
    std::unordered_set<struct file_desc> clients_;
    struct file_desc server_;

    // Binding ID
    int port_;

    // Statistics
    size_t rx_pkt_; // RX: Number of packets read
    size_t tx_pkt_; // TX: Number of packets sent

    bk_server_us_nb(struct manager *mgr);
    virtual ~bk_server_us_nb() override final;

    virtual void start_() override final;
    virtual void stop_() override final;
    virtual size_t get_stats_(char *buf, size_t len) override final;
    virtual int tx_(void *vdata) override final;

    virtual void on_fd_(struct file_desc &fd) override final;
};

#endif // BLOCK_SERVER_US_NB_HPP
