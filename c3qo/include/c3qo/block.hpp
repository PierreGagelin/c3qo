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
    BK_TYPE_NONE = 0,         // Default value
    BK_TYPE_HELLO = 1,        // Block that says hello
    BK_TYPE_CLIENT_US_NB = 2, // Unix stream non-block client
    BK_TYPE_SERVER_US_NB = 3, // Unix stream non-block server
    BK_TYPE_MAX = 4,          // Maximum value
};

//
// @enum bk_cmd
//
// @brief Commands to manage a block
//
// @note Values are used by configuration file any change has an impact on it
//
enum bk_cmd
{
    BK_CMD_NONE = 0,  // Default value
    BK_CMD_ADD = 1,   // Create a block
    BK_CMD_INIT = 2,  // Initialize a block
    BK_CMD_CONF = 3,  // Configure a block
    BK_CMD_BIND = 4,  // Bind a block to another
    BK_CMD_START = 5, // Start a block
    BK_CMD_STOP = 6,  // Stop a block
    BK_CMD_STATS = 7, // Retrieve block's statistics
    BK_CMD_MAX = 8,   // Maximum value
};

//
// @enum bk_state
//
// @brief Block state
//
enum bk_state
{
    BK_STATE_NONE = 0,  // Default value
    BK_STATE_STOP = 1,  // Block is stopped
    BK_STATE_INIT = 2,  // Block is initialized
    BK_STATE_CONF = 3,  // Block is configured
    BK_STATE_START = 4, // Block is started
    BK_STATE_MAX = 5,   // Maximum value
};

//
// @struct bk_if
//
// @brief Declare the interface to manage blocks
//
struct bk_if
{
    // State commands
    void *(*init)();
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
