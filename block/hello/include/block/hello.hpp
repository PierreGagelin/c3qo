#ifndef BLOCK_HELLO_HPP
#define BLOCK_HELLO_HPP

// C++ library headers
#include <cstdlib> // malloc, size_t

// Project headers
#include "c3qo/block.hpp"

//
// @struct hello_bind
//
// @brief Internal structure to bind a port to a block. Here it is
//        a basic array, but it's up to the block to have another way
//        to deal with its bindings (unordered_map, list...)
//
struct hello_bind
{
    int id[8]; // List of binds for this block
};

//
// @struct hello_conf
//
// @brief Internal structure to store configuration. These entries should
//        be provided by the conf callback around startup time
//
struct hello_conf
{
    int bk_id;            // Block ID
    enum bk_type bk_type; // Block type
    char name[64];        // Block name
};

//
// @struct hello_ctx
//
// @brief Internal structure to store the context. To limit arguments in
//        callbacks, it also stores conf and bind structures. It aims
//        to store run-time "context" rather than configuration
//
struct hello_ctx
{
    struct hello_conf conf; // Block configuration structure
    struct hello_bind bind; // Block bind structure

    // Context information
    int count; // Number of packets processed
};

size_t hello_get_stats(void *vctx, char *buf, size_t len);
int hello_ctrl(void *vctx, void *vnotif);
int hello_tx(void *vctx, void *vdata);
int hello_rx(void *vctx, void *vdata);
void hello_stop(void *vctx);
void hello_start(void *vctx);
void hello_bind(void *vctx, int port, int bk_id);
void hello_conf(void *vctx, char *conf);
void *hello_init(int bk_id);

#endif
