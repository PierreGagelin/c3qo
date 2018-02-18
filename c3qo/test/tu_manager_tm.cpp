//
// @brief Test file for the block manager
//

// C++ library headers
#include <string.h> // memcmp, strlen, strncpy

// System library headers
extern "C" {
#include <unistd.h> // sleep
}

// Project headers
#include "c3qo/block.hpp"
#include "c3qo/manager.hpp"
#include "utils/logger.hpp"

// Gtest library
#include "gtest/gtest.h"

// Managers shall be linked
extern struct manager *m;

char zozo_l_asticot[8];
void tm_callback(void *arg)
{
    // We need strlen + 1 bytes to write it
    EXPECT_LT(strlen((char *)arg), sizeof(zozo_l_asticot));

    strncpy(zozo_l_asticot, (char *)arg, sizeof(zozo_l_asticot));
}

// Derive from the manager_tm class
class tu_manager_tm : public testing::Test, public manager_tm
{
  public:
    void SetUp();
    void TearDown();
};

void tu_manager_tm::SetUp()
{
    LOGGER_OPEN("tu_manager_tm");
    logger_set_level(LOGGER_LEVEL_DEBUG);

    // Populate the managers
    m = new struct manager;

    strncpy(zozo_l_asticot, "hello", sizeof("hello"));
}

void tu_manager_tm::TearDown()
{
    // Clear the managers
    delete m;
    
    logger_set_level(LOGGER_LEVEL_NONE);
    LOGGER_CLOSE();
}

//
// @brief Timer expiration
//
TEST_F(tu_manager_tm, manager_tm_expiration)
{
    struct timer t;
    char def[8] = "hello";
    char arg[8] = "world";

    // Initialize manager and zozo
    clear();
    tm_callback(def);

    // Register a 40 ms timer
    t.tid = 0;
    t.callback = &tm_callback;
    t.arg = arg;
    t.time.tv_sec = 0;
    t.time.tv_nsec = 40 * 1000 * 1000;
    EXPECT_EQ(add(t), true);

    // Verify timer expiration
    for (int i = 0; i < 4; i++)
    {
        struct timeval sleep;

        // Sleep 10 ms
        sleep.tv_sec = 0;
        sleep.tv_usec = 10 * 1000;
        EXPECT_EQ(select(0, NULL, NULL, NULL, &sleep), 0);

        // Check timer expiration
        check_exp();

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
TEST_F(tu_manager_tm, manager_tm_order)
{
    struct timer t_0;
    struct timer t_1;
    struct timer t_2;
    struct timespec time_start;
    struct timespec time_cur;
    char arg[3][8] = {"timer0", "timer1", "timer2"};
    char def[8] = "default";

    // Initialize manager and his friend zozo
    clear();
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
    EXPECT_EQ(add(t_2), true);
    EXPECT_EQ(add(t_0), true);
    EXPECT_EQ(add(t_1), true);

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
        check_exp();

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
TEST_F(tu_manager_tm, manager_tm_id)
{
    struct timer t;
    char def[8] = "hello";
    char arg[8] = "world";

    // Initialize manager and the global
    clear();
    tm_callback(def);

    // Register two timers with the same ID:
    //   - 20ms
    //   - 30ms
    t.callback = &tm_callback;
    t.tid = 0;
    t.arg = arg;
    t.time.tv_sec = 0;
    t.time.tv_nsec = 20 * 1000 * 1000;
    EXPECT_EQ(add(t), true);
    t.time.tv_sec = 0;
    t.time.tv_nsec = 30 * 1000 * 1000;
    EXPECT_EQ(add(t), true);

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
        check_exp();

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
