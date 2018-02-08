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

// Manager shall be linked
extern class manager_bk m_bk;

class tu_perf : public testing::Test
{
  public:
    void SetUp();
    void TearDown();
};

void tu_perf::SetUp()
{
    LOGGER_OPEN();
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
    std::unordered_map<int, struct bk_info>::const_iterator bk;
    std::unordered_map<int, struct bk_info>::const_iterator end;
    const char *filename = "/tmp/tu_perf.txt";
    std::fstream file;
    char buf[] = "yolooooo";

    // Reduce amount of output
    logger_set_level(LOGGER_LEVEL_WARNING);

    file.open(filename, std::ios::out | std::ios::trunc);
    ASSERT_EQ(file.is_open(), true);

    // Configure a chain of 100 blocks:
    //   - bk_1 -> bk_2 -> bk_3 -> bk_4... -> bk_100 -> bk_101
    for (int i = 1; i < 101; i++)
    {
        file << CMD_ADD << "   " << i << " " << TYPE_HELLO << std::endl;
        file << CMD_START << " " << i << " no_arg        " << std::endl;

        for (int j = 0; j < 8; j++)
        {
            // Bind port j of bk_i to bk_i+1
            file << CMD_BIND << " " << i << " " << j << ":" << i + 1 << std::endl;
        }
    }

    // Modify binds of bk_100 to point to 0
    for (int j = 0; j < 8; j++)
    {
        file << CMD_BIND << " " << 100 << " " << j << ":0" << std::endl;
    }

    file.close();

    // Parsing configuration
    EXPECT_EQ(m_bk.conf_parse(filename), true);

    // Send data from bk_1
    bk = m_bk.bk_map_.find(1);
    end = m_bk.bk_map_.end();
    ASSERT_TRUE(bk != end);
    for (int i = 0; i < 10000; i++)
    {
        EXPECT_TRUE(bk->second.bk.ctrl(bk->second.ctx, buf) == 0);
    }

    // Verify that 10000 buffers crossed bk_2 to bk_100
    for (int i = 2; i < 101; i++)
    {
        int count;

        bk = m_bk.bk_map_.find(i);
        ASSERT_TRUE(bk != end);

        bk->second.bk.get_stats(bk->second.ctx, buf, sizeof(buf));
        count = atoi(buf);
        EXPECT_TRUE(count == 10000);
    }

    // Clean blocks
    m_bk.block_clear();
}
