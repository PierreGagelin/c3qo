//
// @brief Test file for the block manager
//

// C++ library headers
#include <fstream> // open, close
#include <sstream> // stringstream
#include <stdio.h> // fopen, fileno

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

class tu_manager_bk : public testing::Test, public manager_bk
{
  public:
    void SetUp();
    void TearDown();
};

void tu_manager_bk::SetUp()
{
    LOGGER_OPEN("tu_manager_bk");
    logger_set_level(LOGGER_LEVEL_DEBUG);

    // Populate the managers
    m = new struct manager;
}

void tu_manager_bk::TearDown()
{
    // Clear the managers
    delete m;
    
    logger_set_level(LOGGER_LEVEL_NONE);
    LOGGER_CLOSE();
}

//
// @brief Test the configuration
//
TEST_F(tu_manager_bk, manager_bk_conf)
{
    char fname[] = "/tmp/tu_manager_bk.txt";
    std::fstream file;
    char buf[64];
    std::string buf_exp;
    std::stringstream ss;
    size_t len;

    file.open(fname, std::ios::out | std::ios::trunc);
    ASSERT_EQ(file.is_open(), true);

    // Add, initialize, configure and start 2 blocks hello
    // Spaces should not matter, but 3 values are mandatory
    file << CMD_ADD << "   1          " << TYPE_HELLO << std::endl;
    file << CMD_ADD << "   2          " << TYPE_HELLO << std::endl;
    file << CMD_ADD << "   3          " << TYPE_CLIENT_US_NB << std::endl;
    file << CMD_ADD << "   4          " << TYPE_SERVER_US_NB << std::endl;

    file << CMD_INIT << "  1  no_arg  " << std::endl;
    file << CMD_INIT << "  2  no_arg  " << std::endl;
    file << CMD_INIT << "  3  no_arg  " << std::endl;
    file << CMD_INIT << "  4  no_arg  " << std::endl;

    file << CMD_CONF << "  1  hello_1 " << std::endl;
    file << CMD_CONF << "  2  hello_2 " << std::endl;

    file << CMD_START << " 1  no_arg  " << std::endl;
    file << CMD_START << " 2  no_arg  " << std::endl;
    file << CMD_START << " 3  no_arg  " << std::endl;
    file << CMD_START << " 4  no_arg  " << std::endl;

    // Bindings for block 1:
    //   - port=0 ; bk_id=2
    //   - port=2 ; bk_id=2
    //   - port=4 ; bk_id=2
    //   - port=6 ; bk_id=2
    file << CMD_BIND << " 1  0:2 " << std::endl;
    file << CMD_BIND << " 1  2:2 " << std::endl;
    file << CMD_BIND << " 1  4:2 " << std::endl;
    file << CMD_BIND << " 1  6:2 " << std::endl;

    file.close();

    // Parsing configuration
    EXPECT_EQ(conf_parse(fname), true);

    // Prepare expected configuration dump for the blocks
    //   - format : "<bk_id> <bk_type> <bk_state>;"
    ss << "1 " << TYPE_HELLO << " " << STATE_START << ";";
    ss << "2 " << TYPE_HELLO << " " << STATE_START << ";";
    ss << "3 " << TYPE_CLIENT_US_NB << " " << STATE_START << ";";
    ss << "4 " << TYPE_SERVER_US_NB << " " << STATE_START << ";";
    buf_exp = ss.str();

    // Verify the configuration dump
    len = conf_get(buf, sizeof(buf));
    EXPECT_EQ(len, buf_exp.length());

    // Verify block informations
    for (int i = 1; i < 5; i++)
    {
        const struct bk_info *bi;

        bi = block_get(i);
        ASSERT_NE(bi, (void *)NULL);

        EXPECT_EQ(bi->id, i);
        EXPECT_NE(bi->ctx, (void *)NULL);
        EXPECT_EQ(bi->state, STATE_START);

        switch (i)
        {
        case 1:
        case 2:
            EXPECT_EQ(bi->type, TYPE_HELLO);
            break;

        case 3:
            EXPECT_EQ(bi->type, TYPE_CLIENT_US_NB);
            break;

        case 4:
            EXPECT_EQ(bi->type, TYPE_SERVER_US_NB);
            break;

        default:
            ASSERT_TRUE(false);
            break;
        }
    }

    // Clean blocks
    block_clear();
}

//
// @brief Test the data flow between blocks
//
// For this test, we need to use the statically defined manager of block
//
TEST_F(tu_manager_bk, manager_bk_flow)
{
    const struct bk_info *bk_1;
    const struct bk_info *bk_2;
    char stats[] = "useless value";
    int count;

    // Add, initialize, configure and start 2 blocks
    for (int i = 1; i < 3; i++)
    {
        m->bk.block_add(i, TYPE_HELLO);
        m->bk.block_start(i);
    }

    // Bind:
    //   - block 1 to block 2
    //   - block 2 to block 0 (trash)
    for (int i = 0; i < 8; i++)
    {
        m->bk.block_bind(1, i, 2);
        m->bk.block_bind(2, i, 0);
    }

    // Retrieve block 1 and block 2
    bk_1 = m->bk.block_get(1);
    bk_2 = m->bk.block_get(2);
    ASSERT_NE(bk_1, (void *)NULL);
    ASSERT_NE(bk_2, (void *)NULL);

    // No data should have gone through blocks
    bk_1->bk.get_stats(bk_1->ctx, stats, sizeof(stats));
    count = atoi(stats);
    EXPECT_TRUE(count == 0);
    bk_2->bk.get_stats(bk_2->ctx, stats, sizeof(stats));
    count = atoi(stats);
    EXPECT_TRUE(count == 0);

    // Notify the block to generate a TX data flow: it shall return 0
    EXPECT_TRUE(bk_1->bk.ctrl(bk_1->ctx, stats) == 0);

    // A buffer should have crossed block 2
    bk_1->bk.get_stats(bk_1->ctx, stats, sizeof(stats));
    count = atoi(stats);
    EXPECT_EQ(count, 0);
    bk_2->bk.get_stats(bk_2->ctx, stats, sizeof(stats));
    count = atoi(stats);
    EXPECT_EQ(count, 1);

    // Clear blocks
    m->bk.block_clear();
}
