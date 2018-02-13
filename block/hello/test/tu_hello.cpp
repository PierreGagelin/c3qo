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
extern struct bk_if hello_if;

class tu_hello : public testing::Test
{
    void SetUp();
    void TearDown();
};

void tu_hello::SetUp()
{
    LOGGER_OPEN("tu_hello");
    logger_set_level(LOGGER_LEVEL_DEBUG);
}

void tu_hello::TearDown()
{
    LOGGER_CLOSE();
}

TEST_F(tu_hello, hello)
{
    void *ctx;
    char conf[] = "hello from TU";

    // Should return a context
    ctx = hello_if.init(1);
    EXPECT_TRUE(ctx != NULL);

    // Configure block name
    hello_if.conf(ctx, conf);
    hello_if.conf(NULL, NULL);

    // Start with good and bad value
    hello_if.start(ctx);
    hello_if.start(NULL);

    // Block not binded (or called without context) should return 0 to drop
    for (int i = 0; i < 8; i++)
    {
        EXPECT_TRUE(hello_if.rx(ctx, NULL) == 0);
        EXPECT_TRUE(hello_if.rx(NULL, NULL) == 0);

        EXPECT_TRUE(hello_if.tx(ctx, NULL) == 0);
        EXPECT_TRUE(hello_if.tx(NULL, NULL) == 0);
    }

    // Bind block (0 -> 0, 1 -> 1... 7 -> 7)
    for (int i = 0; i < 8; i++)
    {
        hello_if.bind(NULL, 0, 0); // Should do nothing
        hello_if.bind(ctx, i, i);  // Should do a bind
    }

    // Verify binding (block hello only increment port output)
    for (int i = 0; i < 8; i++)
    {
        EXPECT_TRUE(hello_if.rx(ctx, NULL) == i);
    }
    for (int i = 0; i < 8; i++)
    {
        EXPECT_TRUE(hello_if.tx(ctx, NULL) == i);
    }

    // Stop with good and bad value
    hello_if.stop(ctx);
    hello_if.stop(NULL);
}
