/**
 * @brief Test file for the hello block
 */


#include <stdlib.h>

#include "c3qo/block.hpp"
#include "c3qo/logger.hpp"

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
        hello_entry.ctrl(BK_INIT, NULL);
        hello_entry.ctrl(BK_START, NULL);
}


