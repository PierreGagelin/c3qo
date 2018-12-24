//
// @brief Test file for the hello block
//

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

    block.start_();

    // Verify data and control behaviors
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

int main(int, char **)
{
    LOGGER_OPEN("tu_hello");
    logger_set_level(LOGGER_LEVEL_DEBUG);

    tu_hello_hello();

    LOGGER_CLOSE();
    return 0;
}
