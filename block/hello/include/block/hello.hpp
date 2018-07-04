#ifndef BLOCK_HELLO_HPP
#define BLOCK_HELLO_HPP

// C++ library headers
#include <cstdlib> // malloc, size_t

// Project headers
#include "c3qo/block.hpp"

//
// @struct hello_ctx
//
// @brief Context of the block
//
struct hello_ctx
{
    int bk_id;     // Block ID
    char name[64]; // Block name

    int count; // Number of packets processed
};

extern struct bk_if hello_if;

#endif // BLOCK_HELLO_HPP
