#ifndef _C3PO_BLOCK_H_
#define _C3PO_BLOCK_H_


/**
 * @brief Identifier for each block
 */
enum block_id
{
        HELLOER,  /**< block that says hello */
        GOODBYER, /**< block that says good bye */
};


/**
 * @brief Identifier for events
 */
enum block_cmd
{
        BLOCK_ADD,       /**< Create a block */
        BLOCK_INIT,      /**< Initialize a block */
        BLOCK_CONFIGURE, /**< Configure a block */
        BLOCK_BIND,      /**< Bind a block to another */
        BLOCK_START,     /**< Ask the block to start */
        BLOCK_STOP,      /**< Ask the block to stop */
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


