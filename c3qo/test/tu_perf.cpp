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
#include "c3qo/manager_bk.hpp"
#include "c3qo/manager_fd.hpp"
#include "c3qo/manager_tm.hpp"
#include "utils/logger.hpp"

// Gtest library
#include "gtest/gtest.h"

// Managers shall be linked
extern class manager_bk m_bk;

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
}

void tu_perf::TearDown()
{
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
        m_bk.block_add(i, TYPE_HELLO);
        m_bk.block_start(i);

        for (int j = 0; j < 8; j++)
        {
            // Bind port j of bk_i to bk_i+1
            m_bk.block_bind(i, j, i + 1);
        }
    }

    // Modify binds of bk_100 to point to 0
    for (int j = 0; j < 8; j++)
    {
        m_bk.block_bind(100, j, 0);
    }

    // Send data from bk_1
    bi = m_bk.block_get(1);
    ASSERT_NE(bi, (void *)NULL);
    for (int i = 0; i < 10000; i++)
    {
        EXPECT_EQ(bi->bk.ctrl(bi->ctx, buf), 0);
    }

    // Verify that 10000 buffers crossed bk_2 to bk_100
    for (int i = 2; i < 101; i++)
    {
        int count;

        bi = m_bk.block_get(i);
        ASSERT_NE(bi, (void *)NULL);

        bi->bk.get_stats(bi->ctx, buf, sizeof(buf));
        count = atoi(buf);
        EXPECT_TRUE(count == 10000);
    }

    // Clean blocks
    m_bk.block_clear();
}
