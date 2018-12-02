#ifndef C3QO_BLOCK_HPP
#define C3QO_BLOCK_HPP

// Project headers
#include "utils/logger.hpp"
#include "utils/socket.hpp"

//
// Macro to "register" a block: access to class constructor and destructor via dlopen / dlsym
//
#define BLOCK_REGISTER(name)                              \
    extern "C"                                            \
    {                                                     \
        struct block *name##_create(struct manager *mgr_) \
        {                                                 \
            return new struct name(mgr_);                 \
        }                                                 \
        void name##_destroy(struct block *bk_)            \
        {                                                 \
            delete static_cast<struct name *>(bk_);       \
        }                                                 \
    }

//
// @enum bk_cmd
//
// @brief Commands to manage a block
//
// @note Values are used by configuration file any change has an impact on it
//
enum bk_cmd
{
    CMD_ADD,   // Create a block
    CMD_INIT,  // Initialize a block
    CMD_CONF,  // Configure a block
    CMD_BIND,  // Bind a block to another
    CMD_START, // Start a block
    CMD_STOP,  // Stop a block
    CMD_STATS, // Retrieve block's statistics
};
const char *bk_cmd_to_string(enum bk_cmd t);

//
// @enum bk_state
//
// @brief Block state
//
enum bk_state
{
    STATE_STOP,  // Block is stopped
    STATE_INIT,  // Block is initialized
    STATE_START, // Block is started
};
const char *bk_state_to_string(enum bk_state t);

//
// @enum flow_type
//
enum flow_type
{
    FLOW_RX,    // RX flow
    FLOW_TX,    // TX flow
    FLOW_NOTIF, // Notification flow
};
const char *flow_type_to_string(enum flow_type type);

//
// @struct bind_info
//
// @brief Information to bind to blocks together
//
struct bind_info
{
    int port;         // Port from source block
    int bk_id;        // Identifier of the destination block
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
    file_desc();

    struct block *bk; // Block to be notified on event
    int fd;           // File descriptor to monitor
    void *socket;     // ZMQ socket to monitor
    bool read;        // Look for read events
    bool write;       // Look for write events
};


bool operator==(const struct file_desc &a, const struct file_desc &b);

// custom specialization of std::hash
namespace std
{
template <>
struct hash<struct file_desc>
{
    std::size_t operator()(const struct file_desc &entry) const noexcept;
};
}

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

    block(struct manager *mgr);
    virtual ~block() = 0;

    // Block management interface default implementation
    virtual void init_();
    virtual void conf_(char *conf);
    virtual void bind_(int port, int bk_id);
    virtual void start_();
    virtual void stop_();
    virtual size_t get_stats_(char *buf, size_t len);
    virtual int rx_(void *vdata);
    virtual int tx_(void *vdata);
    virtual int ctrl_(void *vnotif);

    // Timer callback
    virtual void on_timer_(struct timer &tm);

    // File descriptor callback
    virtual void on_fd_(struct file_desc &fd);

    // Data flow methods
    void process_rx_(int port, void *data);
    void process_tx_(int port, void *data);
    void process_notif_(int port, void *notif);
    void process_flow_(int port, void *data, enum flow_type type);
};

#endif // C3QO_BLOCK_HPP
