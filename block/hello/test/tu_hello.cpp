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

    // configure and start the block
    block.conf_(conf);
    block.start_();

    // Verify binding (block hello only increment port output)
    for (int i = 0; i < 8; i++)
    {
        ASSERT(block.rx_(nullptr) == i);
    }
    for (int i = 0; i < 8; i++)
    {
        ASSERT(block.tx_(nullptr) == i);
    }

    // Do not forward notification
    ASSERT(block.ctrl_(nullptr) == 0);

    // Block should count 16 data
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
