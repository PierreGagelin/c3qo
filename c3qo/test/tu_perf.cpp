//
// @brief Test file for the block manager
//

// C++ library headers
#include <fstream>  // open, close
#include <sstream>  // stringstream
#include <stdio.h>  // fopen, fileno
#include <string.h> // memcmp, strlen, strncpy

// System library headers
extern "C" {
#include <unistd.h> // sleep
}

// Project headers
#include "c3qo/block.hpp"
#include "c3qo/manager.hpp"
#include "utils/logger.hpp"

// Gtest library
#include "gtest/gtest.h"

// Managers shall be linked
extern struct manager *m;

class tu_perf : public testing::Test
{
  public:
    void SetUp();
    void TearDown();
};

void tu_perf::SetUp()
{
    LOGGER_OPEN("tu_perf");
    logger_set_level(LOGGER_LEVEL_DEBUG);

    // Populate the managers
    m = new struct manager;
}

void tu_perf::TearDown()
{
    // Clear the managers
    delete m;

    logger_set_level(LOGGER_LEVEL_NONE);
    LOGGER_CLOSE();
}

//
// @brief Test the speed of commutation
//
TEST_F(tu_perf, commutation)
{
    const struct bk_info *bi;
    char buf[] = "yolooooo";

    // Reduce amount of output
    logger_set_level(LOGGER_LEVEL_WARNING);

    // Configure a chain of 100 blocks:
    //   - bk_1 -> bk_2 -> bk_3 -> bk_4... -> bk_100 -> bk_101
    for (int i = 1; i < 101; i++)
    {
        m->bk.block_add(i, TYPE_HELLO);
        m->bk.block_start(i);

        for (int j = 0; j < 8; j++)
        {
            // Bind port j of bk_i to bk_i+1
            m->bk.block_bind(i, j, i + 1);
        }
    }

    // Modify binds of bk_100 to point to 0
    for (int j = 0; j < 8; j++)
    {
        m->bk.block_bind(100, j, 0);
    }

    // Send data from bk_1
    for (int i = 0; i < 10 * 1000; i++)
    {
        m->bk.process_notif(1, buf);
    }

    // Verify that 10 000 buffers crossed bk_2 to bk_100
    for (int i = 2; i < 101; i++)
    {
        int count;

        bi = m->bk.block_get(i);
        ASSERT_NE(bi, (void *)NULL);

        bi->bk.get_stats(bi->ctx, buf, sizeof(buf));
        count = atoi(buf);
        EXPECT_EQ(count,  10 * 1000);
    }

    // Clean blocks
    m->bk.block_clear();
}
