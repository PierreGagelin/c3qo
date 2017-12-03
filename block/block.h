#ifndef _C3PO_BLOCK_H_
#define _C3PO_BLOCK_H_



/**
 * @brief Identifier for each block
 */
enum block_id
{
        HELLOER, /**< block that says hello */
        GOODBYER /**< block that says good bye */
};



/**
 * @brief Identifier for events
 */
enum block_event
{
        BLOCK_INIT, /**< Ask the block to initialize */
        BLOCK_START /**< Ask the block to start */
};

/**
 * @brief Declare the interface to call blocks
 */
struct block_if
{
        /* Configuration management */
        void *conf;
        void (*conf_init) (void);
        void (*conf_set) (void);
        void (*conf_clean) (void);

        /* Context management */
        void *ctx;
        void (*ctx_init) (void);
        void (*ctx_clean) (void);

        /* Statistics management */
        void *stats;
        void (*stats_init) (void);
        void (*stats_get) (void);
        void (*stats_clean) (void);

        /* Data processing */
        void (*rx) (void);
        void (*tx) (void);
        void (*ctrl) (enum block_event, void *);
};

#endif
