#ifndef C3QO_BLOCK_HPP
#define C3QO_BLOCK_HPP

//
// @brief Identifier for block type
//        BK_TYPE_MAX should be the maximum value
//
// @note Values are used by configuration file
//       any change has an impact on it
//
enum bk_type
{
    BK_TYPE_NONE = 0,         // Default value
    BK_TYPE_HELLO = 1,        // Block that says hello
    BK_TYPE_CLIENT_US_NB = 2, // Unix stream non-block client
    BK_TYPE_SERVER_US_NB = 3, // Unix stream non-block server
    BK_TYPE_MAX = 3,          // Maximum value
};

//
// @brief Identifier for events
//        BK_CMD_MAX should be the maximum value
//
// @note Values are used by configuration file
//       any change has an impact on it
//
enum bk_cmd
{
    BK_CMD_NONE = 0,  // Default value
    BK_CMD_ADD = 1,   // Create a block
    BK_CMD_INIT = 2,  // Initialize a block
    BK_CMD_CONF = 3,  // Configure a block
    BK_CMD_BIND = 4,  // Bind a block to another
    BK_CMD_START = 5, // Ask the block to start
    BK_CMD_STOP = 6,  // Ask the block to stop
    BK_CMD_STATS = 7, // Ask the block to stop
    BK_CMD_MAX = 7,   // Maximum value
};

//
// @brief State of the block
//
// @note NONE and MAX values not required as
//       it doesn't come from user input
//
enum bk_state
{
    BK_STATE_STOP = 0,
    BK_STATE_INIT = 1,
    BK_STATE_CONF = 2,
    BK_STATE_START = 3,
};

//
// @brief Different type of block data
//
enum bk_data_type
{
    BK_DATA_TYPE_NONE = 0, // The data is unknown
    BK_DATA_TYPE_BUF = 1,  // The data is a buffer
};

//
// @brief Data that travels through blocks
//
struct bk_data
{
    enum bk_data_type type; // Type of data
    void *data;             // data
};

//
// @brief Data corresponding to BK_DATA_TYPE_BUF
//
struct bk_buf
{
    size_t len; // size of the buffer
    void *buf;  // buffer
};

//
// @brief Declare the interface to manage blocks
//
struct bk_if
{
    // Context
    void *ctx;

    // Get Statistics
    size_t (*stats)(char *buf, size_t len);

    // Data processing
    void (*rx)(struct bk_data *data);
    void (*tx)(struct bk_data *data);
    void (*ctrl)(enum bk_cmd cmd, void *arg);
};

#endif // C3QO_BLOCK_HPP
