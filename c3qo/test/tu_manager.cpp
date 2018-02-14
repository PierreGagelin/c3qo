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

// Managers shall be linked
extern class manager_bk m_bk;
extern class manager_tm m_tm;
extern class manager_fd m_fd;

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
    EXPECT_LT(strlen((char *)arg), sizeof(zozo_l_asticot));

    strncpy(zozo_l_asticot, (char *)arg, sizeof(zozo_l_asticot));
}

class tu_manager : public testing::Test, public manager_bk
{
  public:
    void SetUp();
    void TearDown();
};

void tu_manager::SetUp()
{
    LOGGER_OPEN("tu_manager");
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
// @brief Test the configuration
//
TEST_F(tu_manager, manager_bk_conf)
{
    char fname[] = "/tmp/tu_manager_config_bind.txt";
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
    ss << "1 " << TYPE_HELLO        << " " << STATE_START << ";";
    ss << "2 " << TYPE_HELLO        << " " << STATE_START << ";";
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
TEST_F(tu_manager, manager_bk_flow)
{
    const struct bk_info *bk_1;
    const struct bk_info *bk_2;
    char stats[] = "useless value";
    int count;

    // Add, initialize, configure and start 2 blocks
    for (int i = 1; i < 3; i++)
    {
        m_bk.block_add(i, TYPE_HELLO);
        m_bk.block_start(i);
    }

    // Bind:
    //   - block 1 to block 2
    //   - block 2 to block 0 (trash)
    for (int i = 0; i < 8; i++)
    {
        m_bk.block_bind(1, i, 2);
        m_bk.block_bind(2, i, 0);
    }

    // Retrieve block 1 and block 2
    bk_1 = m_bk.block_get(1);
    bk_2 = m_bk.block_get(2);
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
    m_bk.block_clear();
}

//
// @brief Test the file descriptor manager
//
TEST_F(tu_manager, manager_fd)
{
    char fname[] = "/tmp/tu_manager_fd.txt";
    FILE *file;
    int fd;

    // Open a file and get its file descriptor
    file = fopen(fname, "w+");
    ASSERT_NE(file, (void *)NULL);
    fd = fileno(file);
    ASSERT_NE(fd, -1);

    // Add a file descriptor to be managed for reading
    EXPECT_EQ(m_fd.add(NULL, fd, &fd_callback, true), true);

    // Write into the managed
    fprintf(file, "hello world!");

    // Verify something is ready to be read
    EXPECT_GT(m_fd.select_fd(), 0);

    // Verify that the callback was executed
    EXPECT_EQ(fd_called, true);

    // Clean the file descriptor manager
    m_fd.remove(fd, true);
    fclose(file);
}

//
// @brief Timer expiration
//
TEST_F(tu_manager, manager_tm_expiration)
{
    struct timer t;
    char def[8] = "hello";
    char arg[8] = "world";

    // Initialize manager and zozo
    m_tm.clear();
    tm_callback(def);

    // Register a 40 ms timer
    t.tid = 0;
    t.callback = &tm_callback;
    t.arg = arg;
    t.time.tv_sec = 0;
    t.time.tv_nsec = 40 * 1000 * 1000;
    EXPECT_EQ(m_tm.add(t), true);

    // Verify timer expiration
    for (int i = 0; i < 4; i++)
    {
        struct timeval sleep;

        // Sleep 10 ms
        sleep.tv_sec = 0;
        sleep.tv_usec = 10 * 1000;
        EXPECT_EQ(select(0, NULL, NULL, NULL, &sleep), 0);

        // Check timer expiration
        m_tm.check_exp();

        if (i < 3)
        {
            // Timer shouldn't have expired
            EXPECT_EQ(memcmp(zozo_l_asticot, def, strlen(def) + 1), 0);
        }
        else
        {
            // Timer should have expired
            EXPECT_EQ(memcmp(zozo_l_asticot, arg, strlen(arg) + 1), 0);
        }
    }
}

//
// @brief Timer expiration order
//
TEST_F(tu_manager, manager_tm_order)
{
    struct timer t_0;
    struct timer t_1;
    struct timer t_2;
    struct timespec time_start;
    struct timespec time_cur;
    char arg[3][8] = {"timer0", "timer1", "timer2"};
    char def[8] = "default";

    // Initialize manager and his friend zozo
    m_tm.clear();
    tm_callback(def);

    // Get system time
    ASSERT_NE(clock_gettime(CLOCK_REALTIME, &time_start), -1);

    // Register three timers:
    //   - 40ms
    //   - 80ms
    //   - 120ms
    // Insert them in the wrong order
    t_0.callback = &tm_callback;
    t_0.tid = 0;
    t_0.arg = arg[0];
    t_0.time.tv_sec = 0;
    t_0.time.tv_nsec = 40 * 1000 * 1000;
    t_1.callback = &tm_callback;
    t_1.tid = 1;
    t_1.arg = arg[1];
    t_1.time.tv_sec = 0;
    t_1.time.tv_nsec = 80 * 1000 * 1000;
    t_2.callback = &tm_callback;
    t_2.tid = 2;
    t_2.arg = arg[2];
    t_2.time.tv_sec = 0;
    t_2.time.tv_nsec = 120 * 1000 * 1000;
    EXPECT_EQ(m_tm.add(t_2), true);
    EXPECT_EQ(m_tm.add(t_0), true);
    EXPECT_EQ(m_tm.add(t_1), true);

    // Verify the order of expiration
    for (int i = 0; i < 6; i++)
    {
        struct timeval sleep;
        char *exp;

        // Sleep 20 ms
        sleep.tv_sec = 0;
        sleep.tv_usec = 20 * 1000;
        EXPECT_EQ(select(0, NULL, NULL, NULL, &sleep), 0);

        // Check timer expiration
        ASSERT_NE(clock_gettime(CLOCK_REALTIME, &time_cur), -1);
        m_tm.check_exp();

        ASSERT_LT(time_start, time_cur);

        time_cur.tv_sec -= time_start.tv_sec;
        time_cur.tv_nsec -= time_start.tv_nsec;

        // Wrap microseconds value to be both in range and positive
        time_cur.tv_sec -= time_start.tv_nsec / NSEC_MAX;
        time_cur.tv_nsec = time_start.tv_nsec % NSEC_MAX;

        if (time_cur < t_0.time)
        {
            // Timer shouldn't have expired
            exp = def;
            break;
        }
        else if (time_cur < t_0.time)
        {
            // Timer 0 should expire but not yet timer 1
            exp = arg[0];
        }
        else if (time_cur < t_0.time)
        {
            // Timer 1 should expire but not yet timer 2
            exp = arg[1];
        }
        else
        {
            // Timer 2 should expire
            exp = arg[2];
        }
        EXPECT_EQ(memcmp(zozo_l_asticot, exp, strlen(exp) + 1), 0);
    }
}

//
// @brief Timer identification
//
TEST_F(tu_manager, manager_tm_id)
{
    struct timer t;
    char def[8] = "hello";
    char arg[8] = "world";

    // Initialize manager and the global
    m_tm.clear();
    tm_callback(def);

    // Register two timers with the same ID:
    //   - 20ms
    //   - 30ms
    t.callback = &tm_callback;
    t.tid = 0;
    t.arg = arg;
    t.time.tv_sec = 0;
    t.time.tv_nsec = 20 * 1000 * 1000;
    EXPECT_EQ(m_tm.add(t), true);
    t.time.tv_sec = 0;
    t.time.tv_nsec = 30 * 1000 * 1000;
    EXPECT_EQ(m_tm.add(t), true);

    // Verify only the 30ms is kept
    for (int i = 0; i < 4; i++)
    {
        struct timeval sleep;
        char *exp;

        // Sleep 10 ms
        sleep.tv_sec = 0;
        sleep.tv_usec = 10 * 1000;
        EXPECT_EQ(select(0, NULL, NULL, NULL, &sleep), 0);

        // Check timer expiration
        m_tm.check_exp();

        if (i < 2)
        {
            // Timer shouldn't have expired
            exp = def;
        }
        else
        {
            // Timer should expire
            exp = arg;
        }

        EXPECT_EQ(memcmp(zozo_l_asticot, exp, strlen(exp) + 1), 0);
    }
}
