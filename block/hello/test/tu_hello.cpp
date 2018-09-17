//
// @brief Test file for the hello block
//

#define LOGGER_TAG "[TU.hello]"

// Project headers
#include "block/hello.hpp"
#include "c3qo/manager.hpp"

// Gtest library
#include "gtest/gtest.h"

class tu_hello : public testing::Test
{
    void SetUp();
    void TearDown();

  public:
    struct manager mgr_;
};

void tu_hello::SetUp()
{
    LOGGER_OPEN("tu_hello");
    logger_set_level(LOGGER_LEVEL_DEBUG);
}

void tu_hello::TearDown()
{
    logger_set_level(LOGGER_LEVEL_NONE);
    LOGGER_CLOSE();
}

//
// @brief Basic usage of block hello
//
TEST_F(tu_hello, hello)
{
    struct hello block(&mgr_);
    char conf[] = "hello from TU";
    char stats[] = "useless value";
    int count;

    // configure and start the block
    block.conf_(conf);
    block.start_();

    // Verify binding (block hello only increment port output)
    for (int i = 0; i < 8; i++)
    {
        EXPECT_EQ(block.rx_(nullptr), i);
    }
    for (int i = 0; i < 8; i++)
    {
        EXPECT_EQ(block.tx_(nullptr), i);
    }

    // Do not forward notification
    EXPECT_EQ(block.ctrl_(nullptr), 0);

    // Block should count 16 data (2 characters)
    EXPECT_EQ(block.get_stats_(stats, sizeof(stats)), 2u);
    count = atoi(stats);
    EXPECT_EQ(count, 16);

    // Stop block
    block.stop_();
}

//
// @brief Edge cases
//
TEST_F(tu_hello, error)
{
    struct hello block(&mgr_);
    char stats[] = "lol";

    // We do not want to see ERROR level as it's expected
    logger_set_level(LOGGER_LEVEL_CRIT);

    // Configure without a configuration
    block.conf_(nullptr);

    // Get statistics without buffer
    EXPECT_EQ(block.get_stats_(nullptr, 4u), 0u);
    EXPECT_EQ(block.get_stats_(stats, 0u), 0u);

    // Get statistics with a short buffer
    EXPECT_EQ(block.get_stats_(stats, 1u), 0u);
}
