//
// @brief Test file for the hello block
//

// Project headers
#include "c3qo/manager.hpp"

// Gtest library
#include "gtest/gtest.h"

// Managers shall be linked
extern struct manager *m;

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

    // Populate the managers
    m = new struct manager;
}

void tu_hello::TearDown()
{
    // Clear the managers
    delete m;

    logger_set_level(LOGGER_LEVEL_NONE);
    LOGGER_CLOSE();
}

//
// @brief Basic usage of block hello
//
TEST_F(tu_hello, hello)
{
    void *ctx;
    char conf[] = "hello from TU";
    char stats[] = "useless value";
    int count;

    // Initialize, configure and start the block
    ctx = hello_if.init(1);
    ASSERT_NE(ctx, (void *)NULL);
    hello_if.conf(ctx, conf);
    hello_if.start(ctx);

    // Verify binding (block hello only increment port output)
    for (int i = 0; i < 8; i++)
    {
        EXPECT_EQ(hello_if.rx(ctx, NULL), i);
    }
    for (int i = 0; i < 8; i++)
    {
        EXPECT_EQ(hello_if.tx(ctx, NULL), i);
    }

    // Do not forward notification
    EXPECT_EQ(hello_if.ctrl(ctx, NULL), 0);

    // Block should count 16 data (2 characters)
    EXPECT_EQ(hello_if.get_stats(ctx, stats, sizeof(stats)), (size_t)2);
    count = atoi(stats);
    EXPECT_EQ(count, 16);

    // Stop block
    hello_if.stop(ctx);
}

///
// @brief Edge cases
//
TEST_F(tu_hello, error)
{
    void *ctx;
    char conf[] = "hello";
    char stats[] = "lol";

    // We do not want to see ERROR level as it's expected
    logger_set_level(LOGGER_LEVEL_CRIT);

    ctx = hello_if.init(1);
    ASSERT_NE(ctx, (void *)NULL);

    hello_if.conf(NULL, conf);
    hello_if.conf(ctx, NULL);

    hello_if.start(NULL);

    // Flow without context should return 0 to drop
    EXPECT_EQ(hello_if.rx(NULL, NULL), 0);
    EXPECT_EQ(hello_if.tx(NULL, NULL), 0);
    EXPECT_EQ(hello_if.ctrl(NULL, NULL), 0);

    // Get statistics without buffer or context
    EXPECT_EQ(hello_if.get_stats(NULL, stats, 12), (size_t)0);
    EXPECT_EQ(hello_if.get_stats(ctx, NULL, 12), (size_t)0);
    EXPECT_EQ(hello_if.get_stats(ctx, stats, 0), (size_t)0);

    // Get statistics with a short buffer
    EXPECT_EQ(hello_if.get_stats(ctx, stats, 1), (size_t)0);

    hello_if.stop(NULL);
    hello_if.stop(ctx);
}
