/**
 * @brief Test file for the hello block
 */


#include <stdlib.h>

//#include "block/hello.h"
#include "c3qo/block.h"

#include "gtest/gtest.h"

extern struct block_if hello_entry;

class tu_hello : public testing::Test
{
};


TEST_F(tu_hello, hello)
{
        EXPECT_TRUE(&hello_entry != NULL);

        hello_entry.ctrl(BLOCK_INIT, NULL);
        hello_entry.ctrl(BLOCK_START, NULL);
}


