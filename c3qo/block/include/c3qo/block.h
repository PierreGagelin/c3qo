#ifndef _C3PO_BLOCK_H_
#define _C3PO_BLOCK_H_


/**
 * @brief Identifier for block type
 */
enum block_type
{
        BLOCK_NONE = 0, /**< Default value */

        BLOCK_GOODBYE, /**< block that says goodbye */
        BLOCK_HELLO,   /**< block that says hello */

        BLOCK_TYPE_MAX = 2, /**< Maximum value */
};


/**
 * @brief Identifier for events
 */
enum block_cmd
{
        BLOCK_NOOP = 0,  /**< Default value */

        BLOCK_ADD,       /**< Create a block */
        BLOCK_INIT,      /**< Initialize a block */
        BLOCK_CONFIGURE, /**< Configure a block */
        BLOCK_BIND,      /**< Bind a block to another */
        BLOCK_START,     /**< Ask the block to start */
        BLOCK_STOP,      /**< Ask the block to stop */

        BLOCK_CMD_MAX = 6, /**< Maximum value */
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


