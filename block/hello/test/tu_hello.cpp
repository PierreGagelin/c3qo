//
// @brief Test file for the hello block
//

// Project headers
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
    struct bk_hello block(&mgr_);
    char conf[] = "hello from TU";
    char stats[] = "useless value";
    int count;

    // Initialize, configure and start the block
    block.init_();
    ASSERT_NE(block.ctx_, nullptr);
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

///
// @brief Edge cases
//
TEST_F(tu_hello, error)
{
    struct bk_hello block(&mgr_);
    char conf[] = "hello";
    char stats[] = "lol";

    // We do not want to see ERROR level as it's expected
    logger_set_level(LOGGER_LEVEL_CRIT);

    block.init_();
    ASSERT_NE(block.ctx_, nullptr);

    block.conf_(conf);
    block.conf_(nullptr);

    block.start_();

    // Flow without context should return 0 to drop
    block.stop_();
    EXPECT_EQ(block.rx_(nullptr), 0);
    EXPECT_EQ(block.tx_(nullptr), 0);
    EXPECT_EQ(block.ctrl_(nullptr), 0);

    // Get statistics without buffer or context
    EXPECT_EQ(block.get_stats_(stats, 12), 0u);
    block.init_();
    EXPECT_EQ(block.get_stats_(nullptr, 12), 0u);
    EXPECT_EQ(block.get_stats_(stats, 0), 0u);

    // Get statistics with a short buffer
    EXPECT_EQ(block.get_stats_(stats, 1), 0u);

    block.stop_();
}
