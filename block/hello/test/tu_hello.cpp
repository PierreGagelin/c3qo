//
// @brief Test file for the hello block
//

// C++ library headers
#include <cstdlib> // NULL

//Project headers
#include "c3qo/block.hpp"
#include "utils/logger.hpp"

// Gtest library
#include "gtest/gtest.h"

// TU should be linked with the block
extern struct bk_if hello_entry;

class tu_hello : public testing::Test
{
    void SetUp();
    void TearDown();
};

void tu_hello::SetUp()
{
    LOGGER_OPEN();
    logger_set_level(LOGGER_LEVEL_DEBUG);
}

void tu_hello::TearDown()
{
    LOGGER_CLOSE();
}

TEST_F(tu_hello, hello)
{
    // Normal behavior
    EXPECT_TRUE(hello_entry.init() == NULL);
    hello_entry.start(NULL);
    hello_entry.stop(NULL);

    // Unexpected behavior
    hello_entry.start((void *)42);
    hello_entry.stop((void *)42);
}
