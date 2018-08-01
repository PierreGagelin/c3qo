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

    // Context
    // TODO: replace this shit with a real container
    int fd[SOCKET_FD_MAX]; // File descriptors of the socket
    int fd_count;          // Number of fd in use

    // Statistics
    size_t rx_pkt_count; // RX: Number of packets read
    size_t rx_pkt_bytes; // RX: Total size read
    size_t tx_pkt_count; // TX: Number of packets sent
    size_t tx_pkt_bytes; // TX: Total size sent
};

struct bk_server_us_nb : block
{
    virtual void init_() override final;
    virtual void start_() override final;
    virtual void stop_() override final;
    virtual size_t get_stats_(char *buf, size_t len) override final;
    virtual int tx_(void *vdata) override final;
};

#endif // BLOCK_SERVER_US_NB_HPP
