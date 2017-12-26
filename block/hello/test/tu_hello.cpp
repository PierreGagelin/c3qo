/**
 * @brief Test file for the hello block
 */


#include <stdlib.h>

#include "block/hello.h"

#include "gtest/gtest.h"


class tu_hello : public testing::Test
{
};


TEST_F(tu_hello, hello)
{
        EXPECT_TRUE(&hello_entry != NULL);

        hello_entry.ctrl(BLOCK_INIT, NULL);
        hello_entry.ctrl(BLOCK_START, NULL);
}


