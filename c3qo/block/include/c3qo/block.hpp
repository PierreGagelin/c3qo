#ifndef C3QO_BLOCK_H
#define C3QO_BLOCK_H

//
// @brief : Identifier for block type
//          BK_TYPE_MAX should be the maximum value
//
// @note : Values are used by configuration file
//         any change has an impact on it
//
enum bk_type
{
        BK_NONE = 0,         // Default value
        BK_HELLO = 1,        // Block that says hello
        BK_GOODBYE = 2,      // Block that says goodbye
        BK_CLIENT_US_NB = 3, // Unix stream non-block client
        BK_SERVER_US_NB = 4, // Unix stream non-block server
        BK_TYPE_MAX = 4,     // Maximum value
};

//
// @brief : Identifier for events
//          BK_CMD_MAX should be the maximum value
//
// @note : Values are used by configuration file
//         any change has an impact on it
//
enum bk_cmd
{
        BK_NOOP = 0,      // Default value
        BK_ADD = 1,       // Create a block
        BK_INIT = 2,      // Initialize a block
        BK_CONFIGURE = 3, // Configure a block
        BK_BIND = 4,      // Bind a block to another
        BK_START = 5,     // Ask the block to start
        BK_STOP = 6,      // Ask the block to stop
        BK_STATS = 7,     // Ask the block to stop
        BK_CMD_MAX = 7,   // Maximum value
};

//
// @brief : State of the block
//
enum bk_state
{
        BK_STOPPED = 0,
        BK_INITIALIZED = 1,
        BK_CONFIGURED = 2,
        BK_STARTED = 3,
};

//
// @brief : Different type of block data
//
enum bk_data_type
{
        BK_DATA_TYPE_NONE = 0, // The data is unknown
        BK_DATA_TYPE_BUF = 1,  // The data is a buffer
};

//
// @brief : Data that travel through blocks
//
struct bk_data
{
        enum bk_data_type type; // Type of data
        void *data;             // data
};

//
// @brief : Data corresponding to BK_DATA_TYPE_BUF
//
struct bk_buf
{
        size_t len; // size of the buffer
        void *buf;  // buffer
};

//
// @brief : Declare the interface to manage blocks
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

#endif // C3QO_BLOCK_H
