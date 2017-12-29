#ifndef C3QO_BLOCK_H
#define C3QO_BLOCK_H


/**
 * @brief : Identifier for block type
 *          BLOCK_TYPE_MAX should be the maximum value
 *
 * @note : Values are used by configuration file
 *         any change has an impact on it
 */
enum block_type
{
        BLOCK_NONE     = 0, /* Default value */
        BLOCK_GOODBYE  = 1, /* block that says goodbye */
        BLOCK_HELLO    = 2, /* block that says hello */
        BLOCK_TYPE_MAX = 2, /* Maximum value */
};


/**
 * @brief : Identifier for events
 *          BLOCK_CMD_MAX should be the maximum value
 *
 * @note : Values are used by configuration file
 *         any change has an impact on it
 */
enum block_cmd
{
        BLOCK_NOOP      = 0, /* Default value */
        BLOCK_ADD       = 1, /* Create a block */
        BLOCK_INIT      = 2, /* Initialize a block */
        BLOCK_CONFIGURE = 3, /* Configure a block */
        BLOCK_BIND      = 4, /* Bind a block to another */
        BLOCK_START     = 5, /* Ask the block to start */
        BLOCK_STOP      = 6, /* Ask the block to stop */
        BLOCK_CMD_MAX   = 6, /* Maximum value */
};


/**
 * @brief Declare the interface to manage blocks
 */
struct block_if
{
        /* Context */
        void *ctx;

        /* Data processing */
        void (*rx) (void);
        void (*tx) (void);
        void (*ctrl) (enum block_cmd cmd, void *arg);
};


#endif


