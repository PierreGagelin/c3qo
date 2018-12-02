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

bool fd_called;
void fd_callback(void *ctx, int fd)
{
    (void)fd;
    (void)ctx;

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
    logger_set_level(LOGGER_LEVEL_DEBUG);

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
TEST_F(tu_manager, manager_bk_add)
{
    const char *filename = "/tmp/tu_manager_config.txt";
    std::fstream file;
    char buf[65536];
    std::string buf_exp;
    std::stringstream ss;
    size_t len;

    // This test generates a lot of output
    logger_set_level(LOGGER_LEVEL_WARNING);

    file.open(filename, std::ios::out | std::ios::trunc);
    ASSERT_EQ(file.is_open(), true);

    // Add 2 blocks
    //   - format : "<bk_cmd> <bk_id> <bk_type>\n"
    file << CMD_ADD << " 0 " << TYPE_CLIENT_US_NB << std::endl;
    file << CMD_ADD << " 1 " << TYPE_SERVER_US_NB << std::endl;

    // Add tons of block
    for (int i = 2; i < 5000; i++)
    {
        file << CMD_ADD << " " << i << " " << TYPE_HELLO << std::endl;
    }

    file.close();

    // Parsing configuration
    EXPECT_EQ(m_bk.conf_parse(filename), true);

    // Prepare expected configuration dump for the blocks
    //   - format : "<bk_id> <bk_type> <bk_state>;"
    //
    //ss << "0 " << TYPE_CLIENT_US_NB << " " << STATE_STOP << ";"; Should not appear : block ID 0 is reserved!
    ss << "1 " << TYPE_SERVER_US_NB << " " << STATE_STOP << ";";
    for (int i = 2; i < 5000; i++)
    {
        ss << i << " " << TYPE_HELLO << " " << STATE_STOP << ";";
    }
    buf_exp = ss.str();

    // Verify the configuration dump
    len = m_bk.conf_get(buf, sizeof(buf));
    EXPECT_EQ(len, buf_exp.length());

    // Clean blocks
    m_bk.block_clear();
}

//
// @brief Test the bindings of blocks
//
TEST_F(tu_manager, manager_bk_bind)
{
    const char *filename = "/tmp/tu_manager_config_bind.txt";
    std::fstream file;
    char buf[65536];
    std::string buf_exp;
    std::stringstream ss;
    size_t len;

    file.open(filename, std::ios::out | std::ios::trunc);
    ASSERT_EQ(file.is_open(), true);

    // Add, initialize, configure and start 2 blocks hello
    // Spaces should not matter, but 3 values are mandatory
    file << CMD_ADD << "   1          " << TYPE_HELLO << std::endl;
    file << CMD_ADD << "   2          " << TYPE_HELLO << std::endl;
    file << CMD_INIT << "  1  no_arg  " << std::endl;
    file << CMD_INIT << "  2  no_arg  " << std::endl;
    file << CMD_CONF << "  1  hello_1 " << std::endl;
    file << CMD_CONF << "  2  hello_2 " << std::endl;
    file << CMD_START << " 1  no_arg  " << std::endl;
    file << CMD_START << " 2  no_arg  " << std::endl;

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
    EXPECT_EQ(m_bk.conf_parse(filename), true);

    // Prepare expected configuration dump for the blocks
    //   - format : "<bk_id> <bk_type> <bk_state>;"
    ss << "1 " << TYPE_HELLO << " " << STATE_STOP << ";";
    ss << "0 " << TYPE_HELLO << " " << STATE_STOP << ";";
    buf_exp = ss.str();

    // Verify the configuration dump
    len = m_bk.conf_get(buf, sizeof(buf));
    EXPECT_EQ(len, buf_exp.length());

    // Clean blocks
    m_bk.block_clear();
}

//
// @brief Test the data flow between blocks
//
TEST_F(tu_manager, manager_bk_flow)
{
    std::unordered_map<int, struct bk_info>::const_iterator bk_1;
    std::unordered_map<int, struct bk_info>::const_iterator bk_2;
    std::unordered_map<int, struct bk_info>::const_iterator end;
    const char *filename = "/tmp/tu_manager_config_bind.txt";
    std::fstream file;
    char notif[] = "useless value";
    int count;

    file.open(filename, std::ios::out | std::ios::trunc);
    ASSERT_EQ(file.is_open(), true);

    // Add, initialize, configure and start 2 blocks hello
    // Spaces should not matter, but 3 values are mandatory
    file << CMD_ADD << "   1          " << TYPE_HELLO << std::endl;
    file << CMD_ADD << "   2          " << TYPE_HELLO << std::endl;
    file << CMD_START << " 1  no_arg  " << std::endl;
    file << CMD_START << " 2  no_arg  " << std::endl;

    // Bindings :
    //   - block 1 to block 2
    //   - block 2 to block 0
    for (int i = 0; i < 8; i++)
    {
        file << CMD_BIND << " 1  " << i << ":2 " << std::endl;
        file << CMD_BIND << " 2  " << i << ":0 " << std::endl;
    }

    file.close();

    // Parsing configuration
    EXPECT_EQ(m_bk.conf_parse(filename), true);

    // Retrieve block 1 and block 2
    bk_1 = m_bk.bk_map_.find(1);
    bk_2 = m_bk.bk_map_.find(2);
    end = m_bk.bk_map_.end();
    ASSERT_TRUE(bk_1 != end);
    ASSERT_TRUE(bk_2 != end);

    // No data should have gone through blocks
    bk_1->second.bk.get_stats(bk_1->second.ctx, notif, sizeof(notif));
    count = atoi(notif);
    EXPECT_TRUE(count == 0);
    bk_2->second.bk.get_stats(bk_2->second.ctx, notif, sizeof(notif));
    count = atoi(notif);
    EXPECT_TRUE(count == 0);

    // Notify the block to generate a TX data flow: it shall return 0
    EXPECT_TRUE(bk_1->second.bk.ctrl(bk_1->second.ctx, notif) == 0);

    // A buffer should have crossed block 2
    bk_1->second.bk.get_stats(bk_1->second.ctx, notif, sizeof(notif));
    count = atoi(notif);
    EXPECT_TRUE(count == 0);
    bk_2->second.bk.get_stats(bk_2->second.ctx, notif, sizeof(notif));
    count = atoi(notif);
    EXPECT_TRUE(count == 1);

    // Clear blocks
    m_bk.block_clear();
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
    EXPECT_TRUE(manager_fd::add(NULL, fd, &fd_callback, true) == true);
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
