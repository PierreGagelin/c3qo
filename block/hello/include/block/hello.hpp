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
    int bk_id;     // Block ID
    char name[64]; // Block name
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

extern struct bk_if hello_if;

#endif // BLOCK_HELLO_HPP
