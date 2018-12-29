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

    ASSERT(block.count_ == 0);

    // Verify data and control behaviors
    ASSERT(block.data_(nullptr) == true);
    ASSERT(block.count_ == 1);

    block.ctrl_(nullptr);
    ASSERT(block.count_ == 2);

    // Stop block
    block.stop_();
}

int main(int, char **)
{
    LOGGER_OPEN("tu_hello");

    tu_hello_hello();

    LOGGER_CLOSE();
    return 0;
}
