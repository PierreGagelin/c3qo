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
enum block_event
{
        BLOCK_INIT,  /**< Ask the block to initialize */
        BLOCK_START, /**< Ask the block to start */
        BLOCK_STOP,  /**< Ask the block to stop */
};

/**
 * @brief Declare the interface to call blocks
 */
struct block_if
{
        /* Context management */
        void *ctx;
        void (*ctx_init) (void);
        void (*ctx_clean) (void);

        /* Data processing */
        void (*rx) (void);
        void (*tx) (void);
        void (*ctrl) (enum block_event, void *);
};

#endif
