//
// @brief Test file for the block manager
//

// Project headers
#include "c3qo/manager.hpp"

// Gtest library
#include "gtest/gtest.h"

std::vector<std::string> zozo_l_asticot;
void tm_callback(void *arg)
{
    zozo_l_asticot.push_back(std::string(static_cast<const char *>(arg)));
}

// Derive from the manager_tm class
class tu_manager_tm : public testing::Test, public manager
{
    void SetUp();
    void TearDown();
};

void tu_manager_tm::SetUp()
{
    LOGGER_OPEN("tu_manager_tm");
    logger_set_level(LOGGER_LEVEL_DEBUG);
}

void tu_manager_tm::TearDown()
{
    // Clear the list
    zozo_l_asticot.clear();
    
    logger_set_level(LOGGER_LEVEL_NONE);
    LOGGER_CLOSE();
}

//
// @brief Timer expiration
//
TEST_F(tu_manager_tm, manager_tm_expiration)
{
    struct timer t;
    char arg[8] = "world";

    // Initialize manager
    block_clear();

    // Register a 40 ms timer
    t.tid = 0;
    t.callback = &tm_callback;
    t.arg = arg;
    t.time.tv_sec = 0;
    t.time.tv_nsec = 40 * 1000 * 1000;
    EXPECT_EQ(timer_add(t), true);

    // Verify timer expiration
    for (int i = 0; i < 4; i++)
    {
        struct timeval sleep;

        // Sleep 10 ms
        sleep.tv_sec = 0;
        sleep.tv_usec = 10 * 1000;
        EXPECT_EQ(select(0, nullptr, nullptr, nullptr, &sleep), 0);

        // Check timer expiration
        timer_check_exp();

        if (i < 3)
        {
            // Timer shouldn't have expired but this
            // isn't real-time system, we can't assure it
        }
        else
        {
            // Timer should have expired
            ASSERT_EQ(zozo_l_asticot.size(), 1lu);
            EXPECT_EQ(zozo_l_asticot[0], std::string(arg));
        }
    }
}

//
// @brief Timer expiration order
//
TEST_F(tu_manager_tm, manager_tm_order)
{
    struct timer t_0;
    struct timer t_1;
    struct timer t_2;
    struct timespec time_start;
    struct timespec time_cur;
    char arg[3][8] = {"timer0", "timer1", "timer2"};

    // Initialize manager
    block_clear();

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
    EXPECT_EQ(timer_add(t_2), true);
    EXPECT_EQ(timer_add(t_0), true);
    EXPECT_EQ(timer_add(t_1), true);

    // Verify the order of expiration
    for (int i = 0; i < 6; i++)
    {
        struct timeval sleep;

        // Sleep 20 ms
        sleep.tv_sec = 0;
        sleep.tv_usec = 20 * 1000;
        EXPECT_EQ(select(0, nullptr, nullptr, nullptr, &sleep), 0);

        // Check timer expiration
        ASSERT_NE(clock_gettime(CLOCK_REALTIME, &time_cur), -1);
        timer_check_exp();

        ASSERT_LT(time_start, time_cur);

        time_cur.tv_sec -= time_start.tv_sec;
        time_cur.tv_nsec -= time_start.tv_nsec;

        // Wrap microseconds value to be both in range and positive
        time_cur.tv_sec -= time_start.tv_nsec / NSEC_MAX;
        time_cur.tv_nsec = time_start.tv_nsec % NSEC_MAX;

        if (i < 5)
        {
            // Timers shouldn't have all expired
        }
        else
        {
            // Timers should have expired
            ASSERT_EQ(zozo_l_asticot.size(), 3lu);
            EXPECT_EQ(zozo_l_asticot[0], std::string(arg[0]));
            EXPECT_EQ(zozo_l_asticot[1], std::string(arg[1]));
            EXPECT_EQ(zozo_l_asticot[2], std::string(arg[2]));
        }
    }
}

//
// @brief Timer identification
//
TEST_F(tu_manager_tm, manager_tm_id)
{
    struct timer t;
    char arg[8] = "world";

    // Initialize manager and the global
    block_clear();

    // Register two timers with the same ID:
    //   - 2000ms
    //   - 30ms
    t.callback = &tm_callback;
    t.tid = 0;
    t.arg = arg;
    t.time.tv_sec = 0;
    t.time.tv_nsec = 2 * 1000 * 1000 * 1000;
    EXPECT_EQ(timer_add(t), true);
    t.time.tv_sec = 0;
    t.time.tv_nsec = 30 * 1000 * 1000;
    EXPECT_EQ(timer_add(t), true);

    // Verify only the 30ms is kept
    for (int i = 0; i < 4; i++)
    {
        struct timeval sleep;

        // Sleep 10 ms
        sleep.tv_sec = 0;
        sleep.tv_usec = 10 * 1000;
        EXPECT_EQ(select(0, nullptr, nullptr, nullptr, &sleep), 0);

        // Check timer expiration
        timer_check_exp();

        if (i < 2)
        {
            // Timer shouldn't have expired
        }
        else
        {
            // Timer should expire
            ASSERT_EQ(zozo_l_asticot.size(), 1lu);
            EXPECT_EQ(zozo_l_asticot[0], std::string(arg));
        }
    }
}
