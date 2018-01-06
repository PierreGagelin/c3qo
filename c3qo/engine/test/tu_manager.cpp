/**
 * @brief Test file for the block manager
 */


#include <fstream> // open, close
#include <sstream> // stringstream

extern "C" 
{
#include "c3qo/logger.h"  // LOGGER_OPEN, LOGGER_CLOSE
#include "c3qo/manager.h" // manager_conf_parse
}

#include "gtest/gtest.h"


class tu_manager : public testing::Test
{
        void SetUp();
        void TearDown();
};

void tu_manager::SetUp()
{
        LOGGER_OPEN();
        logger_set_level(LOGGER_LEVEL_INFO);
}

void tu_manager::TearDown()
{
        LOGGER_CLOSE();
}


TEST_F(tu_manager, manager)
{
        const char        *filename = "/tmp/tu_manager_config.txt";
        std::fstream      file;
        char              buf[65536];
        std::string       buf_exp;
        std::stringstream ss;
        size_t            len;

        file.open(filename, std::ios::out | std::ios::trunc);
        ASSERT_EQ(file.is_open(), true);

        // bk_cmd=1 (BK_ADD), bk_id=0, bk_type=1 (BK_HELLO)
        // bk_cmd=1 (BK_ADD), bk_id=1, bk_type=2 (BK_GOODBYE)
        file << "1 0 1\n";
        file << "1 1 2\n";

        // Adding tons of block
        for (int i = 2; i < 5000; i++)
        {
                file << "1 " << i << " 1\n";
        }

        file.close();

        EXPECT_EQ(manager_conf_parse(filename), true);

        // bk_id=0, bk_type=1 (BK_HELLO),   bk_state=0 (BK_STOPPED)
        // bk_id=1, bk_type=2 (BK_GOODBYE), bk_state=0 (BK_STOPPED)
        ss << "0 1 0;1 2 0;";

        // rest of the blocks
        for (int i = 2; i < 5000; i++)
        {
                ss << i << " 1 0;";
        }
        buf_exp = ss.str();

        len = manager_conf_get(buf, sizeof(buf));
        EXPECT_EQ(len, buf_exp.length());
        EXPECT_EQ(memcmp(buf, buf_exp.c_str(), len), 0);
}


