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
    size_t nb_block = 1 * 100;
    size_t nb_buf = 1 * 10 * 1000;

    // Reduce amount of output
    logger_set_level(LOGGER_LEVEL_WARNING);

    // Add, init and start some blocks
    for (size_t i = 1; i < nb_block + 1; i++)
    {
        EXPECT_EQ(m->bk.block_add(i, "hello"), true);
        EXPECT_EQ(m->bk.block_init(i), true);
        EXPECT_EQ(m->bk.block_start(i), true);
    }

    // Configure a chain of N blocks:
    //   - bk_1 -> bk_2 -> bk_3 -> bk_4... -> bk_N
    for (size_t i = 1; i < nb_block; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            // Bind port j of bk_i to bk_i+1
            EXPECT_EQ(m->bk.block_bind(i, j, i + 1), true);
        }
    }

    // Bind the last block to 0 (trash)
    for (int j = 0; j < 8; j++)
    {
        EXPECT_EQ(m->bk.block_bind(nb_block, j, 0), true);
    }

    // Send data from bk_1
    for (size_t i = 0; i < nb_buf; i++)
    {
        const class bk_info *bi;
        char buf[] = "yolooooo";

        bi = m->bk.block_get(1);
        ASSERT_NE(bi, (void *)NULL);

        bi->bk->ctrl(bi->ctx, buf);
    }

    // Verify that buffers crossed bk_2 to the last block
    for (size_t i = 2; i < nb_block + 1; i++)
    {
        const class bk_info *bi;
        char buf[16];
        int count;

        bi = m->bk.block_get(i);
        ASSERT_NE(bi, (void *)NULL);

        bi->bk->get_stats(bi->ctx, buf, sizeof(buf));
        count = atoi(buf);
        EXPECT_EQ(count, nb_buf);
    }

    // Clean blocks
    m->bk.block_clear();
}
