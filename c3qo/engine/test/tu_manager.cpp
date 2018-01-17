/**
 * @brief Test file for the block manager
 */


#include <fstream> // open, close
#include <sstream> // stringstream

extern "C" 
{
#include <stdio.h> // fopen, fileno

#include "c3qo/block.h"      // BK_ADD, BK_HELLO, BK_GOODBYE...
#include "c3qo/logger.h"     // LOGGER_OPEN, LOGGER_CLOSE
#include "c3qo/manager_bk.h" // manager_conf_parse
#include "c3qo/manager_fd.h" // manager_fd_init/clean/add/remove/select
}

#include "gtest/gtest.h"


bool fd_called;
static void fd_callback(int fd)
{
        (void) fd;

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
        logger_set_level(LOGGER_LEVEL_WARNING);

        fd_called = false;
}

void tu_manager::TearDown()
{
        logger_set_level(LOGGER_LEVEL_NONE);
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

        // Add 2 blocks
        //   - format : "<bk_cmd> <bk_id> <bk_type>\n"
        file << BK_ADD << " 0 " << BK_HELLO   << std::endl;
        file << BK_ADD << " 1 " << BK_GOODBYE << std::endl; 

        // Add tons of block
        for (int i = 2; i < 5000; i++)
        {
                file << BK_ADD << " " << i << " " << BK_HELLO << std::endl;
        }

        file.close();

        // Parsing configuration
        EXPECT_EQ(manager_conf_parse(filename), true);

        // Prepare expected configuration dump for the blocks
        //   - format : "<bk_id> <bk_type> <bk_state>;"
        ss << "0 " << BK_HELLO   << " " << BK_STOPPED << ";";
        ss << "1 " << BK_GOODBYE << " " << BK_STOPPED << ";";
        for (int i = 2; i < 5000; i++)
        {
                ss << i << " " << BK_HELLO << " " << BK_STOPPED << ";";
        }
        buf_exp = ss.str();

        // Verify the configuration dump
        len = manager_conf_get(buf, sizeof(buf));
        EXPECT_EQ(len, buf_exp.length());
        EXPECT_EQ(memcmp(buf, buf_exp.c_str(), len), 0);

        // Clean blocks
        manager_block_clean();
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


