//
// @brief Test file for the block manager
//

// c3qo test unit library
#include "c3qo/tu.hpp"

class tu_manager_conf : public testing::Test
{
    void SetUp()
    {
        LOGGER_OPEN("tu_manager_conf");
        logger_set_level(LOGGER_LEVEL_DEBUG);
    }
    void TearDown()
    {
        logger_set_level(LOGGER_LEVEL_NONE);
        LOGGER_CLOSE();
    }

  public:
    struct manager mgr_;
};

//
// @brief Test the configuration
//
TEST_F(tu_manager_conf, conf)
{
    char fname[] = "/tmp/tu_manager_conf.txt";
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
    EXPECT_EQ(mgr_.conf_parse(fname), true);

    // Prepare expected configuration dump for the blocks
    //   - format : "<bk_id> <bk_type> <bk_state>;"
    ss << "1 hello " << STATE_START << ";";
    ss << "2 hello " << STATE_START << ";";
    ss << "3 client_us_nb " << STATE_START << ";";
    ss << "4 server_us_nb " << STATE_START << ";";
    buf_exp = ss.str();

    // Verify the configuration dump
    len = mgr_.conf_get(buf, sizeof(buf));
    EXPECT_EQ(len, buf_exp.length());

    // Verify block informations
    for (int i = 1; i < 5; i++)
    {
        const struct block *bi;

        bi = mgr_.block_get(i);
        ASSERT_NE(bi, nullptr);

        EXPECT_EQ(bi->id_, i);
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
    mgr_.block_clear();
}

//
// @brief Edge cases
//
TEST_F(tu_manager_conf, errors)
{
    char fname[] = "/tmp/tu_manager_conf.txt";
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
        EXPECT_EQ(mgr_.conf_parse(fname), false);
    }
}
