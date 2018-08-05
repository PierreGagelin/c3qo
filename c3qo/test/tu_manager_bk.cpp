//
// @brief Test file for the block manager
//

// Project headers
#include "c3qo/manager.hpp"

// Gtest library
#include "gtest/gtest.h"

class tu_manager_bk : public testing::Test, public manager
{
  public:
    void SetUp();
    void TearDown();
};

void tu_manager_bk::SetUp()
{
    LOGGER_OPEN("tu_manager_bk");
    logger_set_level(LOGGER_LEVEL_DEBUG);
}

void tu_manager_bk::TearDown()
{
    logger_set_level(LOGGER_LEVEL_NONE);
    LOGGER_CLOSE();
}

//
// @brief Test creation and use of default block
//
TEST_F(tu_manager_bk, block)
{
    struct block bk(this);

    bk.init_();
    bk.conf_(nullptr);
    bk.bind_(0, 0);
    bk.start_();
    bk.stop_();
    EXPECT_EQ(bk.get_stats_(nullptr, 0u), 0u);
    EXPECT_EQ(bk.rx_(nullptr), 0);
    EXPECT_EQ(bk.tx_(nullptr), 0);
    EXPECT_EQ(bk.ctrl_(nullptr), 0);
}

//
// @brief Test the configuration
//
TEST_F(tu_manager_bk, manager_bk_conf)
{
    char fname[] = "/tmp/tu_manager_bk.txt";
    std::fstream file;
    char buf[512];
    std::string buf_exp;
    std::stringstream ss;
    size_t len;

    file.open(fname, std::ios::out | std::ios::trunc);
    ASSERT_EQ(file.is_open(), true);

    // Add, initialize, configure and start 2 blocks hello
    // Spaces should not matter, but 3 values are mandatory
    file << CMD_ADD << "   1    hello        " << std::endl;
    file << CMD_ADD << "   2    hello        " << std::endl;
    file << CMD_ADD << "   3    client_us_nb " << std::endl;
    file << CMD_ADD << "   4    server_us_nb " << std::endl;

    file << CMD_INIT << "  1 " << std::endl;
    file << CMD_INIT << "  2 " << std::endl;
    file << CMD_INIT << "  3 " << std::endl;
    file << CMD_INIT << "  4 " << std::endl;

    file << CMD_CONF << "  1  hello_1 " << std::endl;
    file << CMD_CONF << "  2  hello_2 " << std::endl;

    file << CMD_START << " 1 " << std::endl;
    file << CMD_START << " 2 " << std::endl;
    file << CMD_START << " 3 " << std::endl;
    file << CMD_START << " 4 " << std::endl;

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
    ss << "1 hello " << STATE_START << ";";
    ss << "2 hello " << STATE_START << ";";
    ss << "3 client_us_nb " << STATE_START << ";";
    ss << "4 server_us_nb " << STATE_START << ";";
    buf_exp = ss.str();

    // Verify the configuration dump
    len = conf_get(buf, sizeof(buf));
    EXPECT_EQ(len, buf_exp.length());

    // Verify block informations
    for (int i = 1; i < 5; i++)
    {
        const struct block *bi;

        bi = block_get(i);
        ASSERT_NE(bi, nullptr);

        EXPECT_EQ(bi->id_, i);
        EXPECT_NE(bi->ctx_, nullptr);
        EXPECT_EQ(bi->state_, STATE_START);

        switch (i)
        {
        case 1:
        case 2:
            EXPECT_EQ(bi->type_, "hello");
            break;

        case 3:
            EXPECT_EQ(bi->type_, "client_us_nb");
            break;

        case 4:
            EXPECT_EQ(bi->type_, "server_us_nb");
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
    struct block *bk_1;
    struct block *bk_2;
    char stats[] = "useless value";
    int count;

    // Add, initialize and start 2 blocks
    for (int i = 1; i < 3; i++)
    {
        EXPECT_EQ(block_add(i, "hello"), true);
        EXPECT_EQ(block_init(i), true);
        EXPECT_EQ(block_start(i), true);
    }

    // Bind:
    //   - block 1 to block 2
    //   - block 2 to block 0 (trash)
    for (int i = 0; i < 8; i++)
    {
        EXPECT_EQ(block_bind(1, i, 2), true);
        EXPECT_EQ(block_bind(2, i, 0), true);
    }

    // Retrieve block 1 and block 2
    bk_1 = block_get(1);
    bk_2 = block_get(2);
    ASSERT_NE(bk_1, nullptr);
    ASSERT_NE(bk_2, nullptr);

    // No data should have gone through blocks
    bk_1->get_stats_(stats, sizeof(stats));
    count = atoi(stats);
    EXPECT_TRUE(count == 0);
    bk_2->get_stats_(stats, sizeof(stats));
    count = atoi(stats);
    EXPECT_TRUE(count == 0);

    // Notify the block to generate a TX data flow: it shall return 0
    EXPECT_TRUE(bk_1->ctrl_(stats) == 0);

    // A buffer should have crossed block 2
    bk_1->get_stats_(stats, sizeof(stats));
    count = atoi(stats);
    EXPECT_EQ(count, 0);
    bk_2->get_stats_(stats, sizeof(stats));
    count = atoi(stats);
    EXPECT_EQ(count, 1);

    // Clear blocks
    block_clear();
}

//
// @brief String version of the block enumerates
//
TEST_F(tu_manager_bk, strings)
{
    for (int i = 0; i < 10; i++)
    {
        bk_cmd_to_string((enum bk_cmd)i);
        bk_state_to_string((enum bk_state)i);
    }
}

//
// @brief Edge cases
//
TEST_F(tu_manager_bk, errors)
{
    char fname[] = "/tmp/tu_manager_bk.txt";
    std::fstream file;

    // Do not display error messages as we known there will be
    logger_set_level(LOGGER_LEVEL_CRIT);

    for (int i = 0; i < 8; i++)
    {
        file.open(fname, std::ios::out | std::ios::trunc);
        ASSERT_EQ(file.is_open(), true);

        switch (i)
        {
        case 0:
            // Undefined block ID
            file << CMD_ADD << " 0 hello" << std::endl;
            break;

        case 1:
            // Undefined block type
            file << CMD_ADD << " 1 " << 42 << std::endl;
            break;

        case 2:
            // Initialize an unexisting block
            file << CMD_INIT << " 4 no_arg" << std::endl;
            break;

        case 3:
            // Configure an unexisting block
            file << CMD_CONF << " 4 hello_1" << std::endl;
            break;

        case 4:
            // Bind an unexisting block
            file << CMD_BIND << " 4  0:2" << std::endl;
            break;

        case 5:
            // Start an unexisting block
            file << CMD_START << " 4 no_arg" << std::endl;
            break;

        case 6:
            // Too many entries
            file << "I like to try out fancy entries in configuration" << std::endl;
            break;

        case 7:
            // Not enough entries
            file << "Both ways " << std::endl;
            break;

        default:
            // Incorrect test
            ASSERT_TRUE(false);
            break;
        }

        file.close();
        EXPECT_EQ(conf_parse(fname), false);
    }
}
