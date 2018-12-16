//
// @brief Test file for the hello block
//

#define LOGGER_TAG "[TU.hello]"

// Project headers
#include "block/hello.hpp"
#include "engine/tu.hpp"

struct manager mgr_;

//
// @brief Basic usage of block hello
//
static void tu_hello_hello()
{
    struct hello block(&mgr_);
    char conf[] = "hello from TU";

    // Configure and start the block
    block.conf_(conf);
    block.start_();

    // Verify output
    for (int i = 1; i < 9; i++)
    {
        ASSERT(block.data_(nullptr) == i);
    }
    ASSERT(block.count_ == 8);
    for (int i = 0; i < 8; i++)
    {
        block.ctrl_(nullptr);
    }
    ASSERT(block.count_ == 16);

    // Stop block
    block.stop_();
}

//
// @brief Edge cases
//
static void tu_hello_error()
{
    struct hello block(&mgr_);

    // We do not want to see ERROR level as it's expected
    logger_set_level(LOGGER_LEVEL_CRIT);

    // Configure without a configuration
    block.conf_(nullptr);

    logger_set_level(LOGGER_LEVEL_DEBUG);
}

int main(int, char **)
{
    LOGGER_OPEN("tu_hello");
    logger_set_level(LOGGER_LEVEL_DEBUG);

    tu_hello_hello();
    tu_hello_error();

    LOGGER_CLOSE();
    return 0;
}
