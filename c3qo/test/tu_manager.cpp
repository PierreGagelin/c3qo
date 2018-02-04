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

#include "gtest/gtest.h"

bool fd_called;
void fd_callback(int fd)
{
    (void)fd;

    fd_called = true;
}

char zozo_l_asticot[8];
void tm_callback(void *arg)
{
    // We need strlen + 1 bytes to write it
    EXPECT_TRUE(strlen((char *)arg) < sizeof(zozo_l_asticot));

    strncpy(zozo_l_asticot, (char *)arg, sizeof(zozo_l_asticot));
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
    strncpy(zozo_l_asticot, "hello", sizeof("hello"));
}

void tu_manager::TearDown()
{
    logger_set_level(LOGGER_LEVEL_NONE);
    LOGGER_CLOSE();
}

//
// @brief Test the block manager
//
TEST_F(tu_manager, manager_bk)
{
    const char *filename = "/tmp/tu_manager_config.txt";
    std::fstream file;
    char buf[65536];
    std::string buf_exp;
    std::stringstream ss;
    size_t len;

    file.open(filename, std::ios::out | std::ios::trunc);
    ASSERT_EQ(file.is_open(), true);

    // Add 2 blocks
    //   - format : "<bk_cmd> <bk_id> <bk_type>\n"
    file << BK_CMD_ADD << " 0 " << BK_TYPE_CLIENT_US_NB << std::endl;
    file << BK_CMD_ADD << " 1 " << BK_TYPE_SERVER_US_NB << std::endl;

    // Add tons of block
    for (int i = 2; i < 5000; i++)
    {
        file << BK_CMD_ADD << " " << i << " " << BK_TYPE_HELLO << std::endl;
    }

    file.close();

    // Parsing configuration
    EXPECT_EQ(manager_bk::conf_parse(filename), true);

    // Prepare expected configuration dump for the blocks
    //   - format : "<bk_id> <bk_type> <bk_state>;"
    ss << "0 " << BK_TYPE_CLIENT_US_NB << " " << BK_STATE_STOP << ";";
    ss << "1 " << BK_TYPE_SERVER_US_NB << " " << BK_STATE_STOP << ";";
    for (int i = 2; i < 5000; i++)
    {
        ss << i << " " << BK_TYPE_HELLO << " " << BK_STATE_STOP << ";";
    }
    buf_exp = ss.str();

    // Verify the configuration dump
    len = manager_bk::conf_get(buf, sizeof(buf));
    EXPECT_EQ(len, buf_exp.length());

    // Clean blocks
    manager_bk::block_clean();
}

//
// @brief Test the file descriptor manager
//
TEST_F(tu_manager, manager_fd)
{
    const char *filename = "/tmp/tu_manager_fd.txt";
    FILE *file;
    int fd;

    // Open a file and get its file descriptor
    file = fopen(filename, "w+");
    ASSERT_TRUE(file != NULL);
    fd = fileno(file);
    ASSERT_TRUE(fd != -1);

    // Add and write to the file
    manager_fd::init();
    EXPECT_TRUE(manager_fd::add(fd, &fd_callback, true) == true);
    fprintf(file, "hello world!");

    // Verify something is ready to be read and callback is called
    EXPECT_TRUE(manager_fd::select() > 0);
    EXPECT_TRUE(fd_called == true);

    manager_fd::remove(fd, true);
    manager_fd::clean();
    fclose(file);
}

//
// @brief Timer expiration
//
TEST_F(tu_manager, manager_tm_expiration)
{
    struct manager_tm::timer t;
    char def[8] = "hello";
    char arg[8] = "world";
    int i;

    // Initialize manager and zozo
    manager_tm::clear();
    tm_callback(def);

    // Register a 200 ms timer
    t.tid = 0;
    t.callback = &tm_callback;
    t.arg = arg;
    t.time.tv_sec = 0;
    t.time.tv_usec = 200000;
    EXPECT_TRUE(manager_tm::add(t) == true);

    // Verify timer expiration
    i = 0;
    while (true)
    {
        struct timeval sleep;

        // Sleep 50 ms
        sleep.tv_sec = 0;
        sleep.tv_usec = 50000;
        EXPECT_TRUE(select(0, NULL, NULL, NULL, &sleep) == 0);
        i++;

        // Check timer expiration
        manager_tm::check_exp();

        if (i < 4)
        {
            // Timer shouldn't have expired
            EXPECT_TRUE(memcmp(zozo_l_asticot, arg, strlen(arg) + 1) != 0);
        }
        else
        {
            // Timer should have expired
            EXPECT_TRUE(memcmp(zozo_l_asticot, arg, strlen(arg) + 1) == 0);
            break;
        }
    }
}

//
// @brief Timer expiration order
//
TEST_F(tu_manager, manager_tm_order)
{
    struct manager_tm::timer t;
    char arg[3][8] = {"timer0", "timer1", "timer2"};
    char def[8] = "default";

    // Initialize manager and zozo
    manager_tm::clear();
    tm_callback(def);

    // Register three timers:
    //   - 200ms
    //   - 300ms
    //   - 100ms
    t.callback = &tm_callback;
    t.tid = 0;
    t.arg = arg[0];
    t.time.tv_sec = 0;
    t.time.tv_usec = 200000;
    EXPECT_TRUE(manager_tm::add(t) == true);
    t.tid = 1;
    t.arg = arg[1];
    t.time.tv_sec = 0;
    t.time.tv_usec = 300000;
    EXPECT_TRUE(manager_tm::add(t) == true);
    t.tid = 2;
    t.arg = arg[2];
    t.time.tv_sec = 0;
    t.time.tv_usec = 100000;
    EXPECT_TRUE(manager_tm::add(t) == true);

    // Verify the order of expiration
    for (int i = 0; i < 6; i++)
    {
        struct timeval sleep;
        char *exp;

        // Sleep 50 ms
        sleep.tv_sec = 0;
        sleep.tv_usec = 50000;
        EXPECT_TRUE(select(0, NULL, NULL, NULL, &sleep) == 0);

        // Check timer expiration
        manager_tm::check_exp();
        switch (i)
        {
        case 0:
            // Timer shouldn't have expired
            exp = def;
            break;

        case 1:
        case 2:
            // Timer 2 should expire but not yet timer 0
            exp = arg[2];
            break;

        case 3:
        case 4:
            // Timer 0 should expire but not yet timer 1
            exp = arg[0];
            break;

        case 5:
            // Timer 1 should expire
            exp = arg[1];
            break;

        default:
            // Problem in the TU
            ASSERT_TRUE(false);
            break;
        }
        EXPECT_TRUE(memcmp(zozo_l_asticot, exp, strlen(exp) + 1) == 0);
    }
}

//
// @brief Timer identification
//
TEST_F(tu_manager, manager_tm_id)
{
    struct manager_tm::timer t;
    char def[8] = "hello";
    char arg[8] = "world";

    // Initialize manager and the global
    manager_tm::clear();
    tm_callback(def);

    // Register two timers with the same ID:
    //   - 200ms
    //   - 300ms
    t.callback = &tm_callback;
    t.tid = 0;
    t.arg = arg;
    t.time.tv_sec = 0;
    t.time.tv_usec = 200000;
    EXPECT_TRUE(manager_tm::add(t) == true);
    t.time.tv_sec = 0;
    t.time.tv_usec = 300000;
    EXPECT_TRUE(manager_tm::add(t) == true);

    // Verify only the 300ms is kept
    for (int i = 0; i < 6; i++)
    {
        struct timeval sleep;
        char *exp;

        // Sleep 50 ms
        sleep.tv_sec = 0;
        sleep.tv_usec = 50000;
        EXPECT_TRUE(select(0, NULL, NULL, NULL, &sleep) == 0);

        // Check timer expiration
        manager_tm::check_exp();
        switch (i)
        {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
            // Timer shouldn't have expired
            exp = def;
            break;

        case 5:
            // Timer should expire
            exp = arg;
            break;

        default:
            // Problem in the TU
            ASSERT_TRUE(false);
            break;
        }
        EXPECT_TRUE(memcmp(zozo_l_asticot, exp, strlen(exp) + 1) == 0);
    }
}
