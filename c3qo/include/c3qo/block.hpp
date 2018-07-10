#ifndef C3QO_BLOCK_HPP
#define C3QO_BLOCK_HPP

// C++ library headers
#include <cstdlib> // size_t

struct bk_if *get_bk_if(const char *b);

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
const char *get_bk_cmd(enum bk_cmd t);

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
    int (*ctrl)(void *vctx, void *vnotif);
};

#endif // C3QO_BLOCK_HPP
