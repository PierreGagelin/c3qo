#ifndef BLOCK_HPP
#define BLOCK_HPP

// Project headers
#include "utils/logger.hpp"

// Port value to return in order to stop a data flow
#define PORT_STOP 0

//
// @enum bk_state
//
// @brief Block state
//
enum bk_state
{
    STATE_STOP,  // Block is stopped
    STATE_START, // Block is started
};

//
// @struct bind_info
//
// @brief Information to bind to blocks together
//
struct bind_info
{
    int port;         // Port from source block
    struct block *bk; // Destination block
};

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
    std::vector<struct bind_info> binds_; // Block bindings
    int id_;                              // Block ID
    std::string type_;                    // Block type
    enum bk_state state_;                 // Block state

    struct manager *mgr_; // Manager of this block

    explicit block(struct manager *mgr);
    virtual ~block();

    // Management callbacks
    virtual void bind_(int port, int bk_id);
    virtual void start_();
    virtual void stop_();

    // Timer callback
    virtual void on_timer_(struct timer &tm);

    // File descriptor callback
    virtual void on_fd_(struct file_desc &fd);

    // Data callbacks
    virtual int data_(void *vdata);
    virtual void ctrl_(void *vnotif);

    // Flow methods
    void process_data_(int port, void *data);
    void process_ctrl_(int port, void *notif);
};

// Factory to create and destroy blocks
struct block_factory
{
    virtual struct block *constructor(struct manager *) = 0;
    virtual void destructor(struct block *) = 0;
};

#endif // BLOCK_HPP
