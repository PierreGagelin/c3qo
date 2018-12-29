#ifndef BLOCK_HPP
#define BLOCK_HPP

// Project headers
#include "utils/logger.hpp"

//
// @struct timer
//
struct timer
{
    struct block *bk;     // Block to be notified on expiration
    struct timespec time; // Expiration date (user specify a delay and it's converted to a date)
    void *arg;            // Generic argument to forward on expiration
    int tid;              // Timer identifier
};

//
// @struct file_desc
//
struct file_desc
{
    struct block *bk; // Block to be notified on event
    int fd;           // File descriptor to monitor
    void *socket;     // ZMQ socket to monitor
    bool read;        // Look for read events
    bool write;       // Look for write events
};

//
// @struct block
//
// @brief Block information
//
struct block
{
    int id_;             // Block ID
    std::string type_;   // Block type
    bool is_started_;    // Block state
    struct block *sink_; // Block bindings

    struct manager *mgr_; // Manager of this block

    explicit block(struct manager *mgr);
    virtual ~block();

    // Management callbacks
    virtual void bind_(int port, struct block *bk);
    virtual void start_();
    virtual void stop_();

    // Timer callback
    virtual void on_timer_(struct timer &tm);

    // File descriptor callback
    virtual void on_fd_(struct file_desc &fd);

    // Data callbacks
    virtual bool data_(void *data);
    virtual void ctrl_(void *notif);

    // Flow methods
    void process_data_(void *data);
    void process_ctrl_(int bk_id, void *notif);
};

// Factory to create and destroy blocks
struct block_factory
{
    virtual struct block *constructor(struct manager *) = 0;
    virtual void destructor(struct block *) = 0;
};

#endif // BLOCK_HPP
