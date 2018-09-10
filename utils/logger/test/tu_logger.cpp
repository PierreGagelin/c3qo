//
// @brief Test file for the block manager
//

#define LOGGER_TAG "[TU.logger]"

// Project headers
#include "utils/logger.hpp"

// Gtest library
#include "gtest/gtest.h"

// Logger library should be linked
extern enum logger_level logger_level;

class tu_logger : public testing::Test
{
  public:
    void SetUp();
    void TearDown();
};

void tu_logger::SetUp()
{
    LOGGER_OPEN("tu_logger");
    logger_set_level(LOGGER_LEVEL_DEBUG);
}

void tu_logger::TearDown()
{   
    logger_set_level(LOGGER_LEVEL_NONE);
    LOGGER_CLOSE();
}

//
// @brief Test level of log
//
TEST_F(tu_logger, level)
{
    // Regular level of logs
    logger_set_level(LOGGER_LEVEL_DEBUG);
    EXPECT_EQ(logger_level, LOGGER_LEVEL_DEBUG);

    logger_set_level(LOGGER_LEVEL_INFO);
    EXPECT_EQ(logger_level, LOGGER_LEVEL_INFO);

    logger_set_level(LOGGER_LEVEL_NOTICE);
    EXPECT_EQ(logger_level, LOGGER_LEVEL_NOTICE);

    logger_set_level(LOGGER_LEVEL_WARNING);
    EXPECT_EQ(logger_level, LOGGER_LEVEL_WARNING);

    logger_set_level(LOGGER_LEVEL_ERR);
    EXPECT_EQ(logger_level, LOGGER_LEVEL_ERR);

    logger_set_level(LOGGER_LEVEL_CRIT);
    EXPECT_EQ(logger_level, LOGGER_LEVEL_CRIT);

    logger_set_level(LOGGER_LEVEL_ALERT);
    EXPECT_EQ(logger_level, LOGGER_LEVEL_ALERT);

    logger_set_level(LOGGER_LEVEL_EMERG);
    EXPECT_EQ(logger_level, LOGGER_LEVEL_EMERG);

    logger_set_level(LOGGER_LEVEL_NONE);
    EXPECT_EQ(logger_level, LOGGER_LEVEL_NONE);

    // Corrupted levels
    logger_set_level((enum logger_level)666);
    EXPECT_EQ(logger_level, LOGGER_LEVEL_DEBUG);

    logger_set_level((enum logger_level)-666);
    EXPECT_EQ(logger_level, LOGGER_LEVEL_NONE);
}
