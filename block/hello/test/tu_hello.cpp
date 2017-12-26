/**
 * @brief Test file for the hello block
 */


#include <stdlib.h>

#include "block/hello.h"

#include "gtest/gtest.h"


int main (int argc, char **argv)
{
        (void) argc;
        (void) argv;

        hello_entry.ctrl(BLOCK_INIT, NULL);
        hello_entry.ctrl(BLOCK_START, NULL);

        EXPECT_EQ(1, 1);

        exit(EXIT_SUCCESS);
}



