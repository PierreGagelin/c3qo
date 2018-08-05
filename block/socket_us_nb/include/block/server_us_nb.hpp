#ifndef BLOCK_SERVER_US_NB_HPP
#define BLOCK_SERVER_US_NB_HPP

// Project headers
#include "c3qo/block.hpp"

#define SOCKET_FD_MAX 64 // Maximum number of file descriptors

//
// @struct server_us_nb_ctx
//
struct server_us_nb_ctx
{
    // Configuration
    int bk_id; // Block ID
    int bind;  // Binding ID

    // Statistics
    size_t rx_pkt_count; // RX: Number of packets read
    size_t rx_pkt_bytes; // RX: Total size read
    size_t tx_pkt_count; // TX: Number of packets sent
    size_t tx_pkt_bytes; // TX: Total size sent
};

struct bk_server_us_nb : block
{
    // File descriptors in use
    std::unordered_set<int> clients_;
    int server_;

    bk_server_us_nb(struct manager *mgr);

    int server_us_nb_fd_find(int fd);
    void server_us_nb_remove(int i);

    virtual void init_() override final;
    virtual void start_() override final;
    virtual void stop_() override final;
    virtual size_t get_stats_(char *buf, size_t len) override final;
    virtual int tx_(void *vdata) override final;
};

#endif // BLOCK_SERVER_US_NB_HPP
