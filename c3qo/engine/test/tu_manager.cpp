/**
 * @brief Test file for the block manager
 */


#include <fstream>  // open, close

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
}

void tu_manager::TearDown()
{
        LOGGER_CLOSE();
}


TEST_F(tu_manager, manager)
{
        const char   *filename = "/tmp/tu_manager_config.txt";
        std::fstream file;
        char         buf[512];
        const char   *buf_exp;
        size_t       len;

        file.open(filename, std::ios::out | std::ios::trunc);
        ASSERT_EQ(file.is_open(), true);

        // bk_cmd=1 (BK_ADD), bk_id=0, bk_type=1 (BK_HELLO)
        // bk_cmd=1 (BK_ADD), bk_id=1, bk_type=2 (BK_GOODBYE)
        file << "1 0 1\n";
        file << "1 1 2\n";

        file.close();

        EXPECT_EQ(manager_conf_parse(filename), true);

        // bk_id=0, bk_type=1 (BK_HELLO),   bk_state=0 (BK_STOPPED)
        // bk_id=1, bk_type=2 (BK_GOODBYE), bk_state=0 (BK_STOPPED)
        buf_exp = "0 1 0;1 2 0;";

        len = manager_conf_get(buf, sizeof(buf));
        EXPECT_EQ(len, strlen(buf_exp));
        EXPECT_EQ(memcmp(buf, buf_exp, len), 0);
}


