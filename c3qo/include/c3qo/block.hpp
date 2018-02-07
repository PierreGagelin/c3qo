#ifndef C3QO_BLOCK_HPP
#define C3QO_BLOCK_HPP

// C++ library headers
#include <cstdlib> // size_t

//
// @enum bk_type
//
// @brief Block types
//
// @note Values are used by configuration file any change has an impact on it
//
enum bk_type
{
    TYPE_NONE = 0,         // Default value
    TYPE_HELLO = 1,        // Block that says hello
    TYPE_CLIENT_US_NB = 2, // Unix stream non-block client
    TYPE_SERVER_US_NB = 3, // Unix stream non-block server
    TYPE_MAX = 4,          // Maximum value
};
const char *get_bk_type(enum bk_type t);

//
// @enum bk_cmd
//
// @brief Commands to manage a block
//
// @note Values are used by configuration file any change has an impact on it
//
enum bk_cmd
{
    CMD_NONE = 0,  // Default value
    CMD_ADD = 1,   // Create a block
    CMD_INIT = 2,  // Initialize a block
    CMD_CONF = 3,  // Configure a block
    CMD_BIND = 4,  // Bind a block to another
    CMD_START = 5, // Start a block
    CMD_STOP = 6,  // Stop a block
    CMD_STATS = 7, // Retrieve block's statistics
    CMD_MAX = 8,   // Maximum value
};
const char *get_bk_cmd(enum bk_cmd t);

//
// @enum bk_state
//
// @brief Block state
//
enum bk_state
{
    STATE_NONE = 0,  // Default value
    STATE_STOP = 1,  // Block is stopped
    STATE_INIT = 2,  // Block is initialized
    STATE_START = 3, // Block is started
    STATE_MAX = 4,   // Maximum value
};
const char *get_bk_state(enum bk_state t);

//
// @struct bk_if
//
// @brief Declare the interface to manage blocks
//
struct bk_if
{
    // State commands
    void *(*init)(int bk_id);
    void (*conf)(void *vctx, char *conf);
    void (*bind)(void *vctx, int port, int bk_id);
    void (*start)(void *vctx);
    void (*stop)(void *vctx);

    // Get Statistics
    size_t (*get_stats)(void *vctx, char *buf, size_t len);

    // Data and control
    int (*rx)(void *vctx, void *vdata);
    int (*tx)(void *vctx, void *vdata);
    void (*ctrl)(void *vctx, void *vnotif);
};

#endif // C3QO_BLOCK_HPP
