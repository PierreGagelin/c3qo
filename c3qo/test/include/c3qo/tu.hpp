#ifndef TU_HPP
#define TU_HPP

// Project headers
#include "c3qo/manager.hpp"

// Gtest library
#include "gtest/gtest.h"

// Generic purpose block structure
struct block_derived : block
{
    block_derived(struct manager *mgr) : block(mgr) {}
    ~block_derived() {}
};

BLOCK_REGISTER(block_derived);

#endif // TU_HPP
