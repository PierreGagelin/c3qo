/**
 * @brief Test file for the block manager
 */


#include <fstream> // open, close
#include <sstream> // stringstream

extern "C" 
{
#include <stdio.h> // fopen, fileno

#include "c3qo/logger.h"     // LOGGER_OPEN, LOGGER_CLOSE
#include "c3qo/manager_bk.h" // manager_conf_parse
#include "c3qo/manager_fd.h" // manager_fd_init/clean/add/remove/select
}

#include "gtest/gtest.h"


bool fd_called;
static void fd_callback(int fd)
{
        fd_called = true;
}


class tu_manager : public testing::Test
{
public:
        void SetUp();
        void TearDown();
};

void tu_manager::SetUp()
{
        LOGGER_OPEN();
        logger_set_level(LOGGER_LEVEL_INFO);

        fd_called = false;
}

void tu_manager::TearDown()
{
        LOGGER_CLOSE();
}


/**
 * @brief Test the block manager
 */
TEST_F(tu_manager, manager_bk)
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


/**
 * @brief Test the file descriptor manager
 */
TEST_F(tu_manager, manager_fd)
{
        const char *filename = "/tmp/tu_manager_fd.txt";
        FILE       *file;
        int        fd;

        // Open a file and get its file descriptor
        file = fopen(filename, "w+");
        ASSERT_TRUE(file != NULL);
        fd   = fileno(file);
        ASSERT_TRUE(fd != -1);

        // Add and write to the file
        manager_fd_init();
        EXPECT_TRUE(manager_fd_add(fd, &fd_callback) == true);
        fprintf(file, "hello world!");

        // Verify that callback was called
        manager_fd_select();
        EXPECT_TRUE(fd_called == true);

        manager_fd_remove(fd);
        manager_fd_clean();
        fclose(file);
}


