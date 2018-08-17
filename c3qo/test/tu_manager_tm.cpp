//
// @brief Test file for the block manager
//

// Project headers
#include "c3qo/tu.hpp"

struct block_timer : block
{
    std::vector<std::string> zozo_l_asticot_;

    block_timer(struct manager *mgr) : block(mgr) {}

    virtual void on_timer_(struct timer &tm) override final
    {
        zozo_l_asticot_.push_back(std::string(static_cast<const char *>(tm.arg)));
    }
};

// Derive from the manager_tm class
class tu_manager_tm : public testing::Test
{
    void SetUp()
    {
        LOGGER_OPEN("tu_manager_tm");
        logger_set_level(LOGGER_LEVEL_DEBUG);
    }
    void TearDown()
    {
        // Clear the list
        block_.zozo_l_asticot_.clear();
        
        logger_set_level(LOGGER_LEVEL_NONE);
        LOGGER_CLOSE();
    }

public:
    struct manager mgr_;
    struct block_timer block_;

    tu_manager_tm() : block_(&mgr_) {}
};

//
// @brief Timer expiration
//
TEST_F(tu_manager_tm, expiration)
{
    struct timer t;
    char arg[8] = "world";

    // Register a 40 ms timer
    t.tid = 0;
    t.bk = &block_;
    t.arg = arg;
    t.time.tv_sec = 0;
    t.time.tv_nsec = 40 * 1000 * 1000;
    EXPECT_EQ(mgr_.timer_add(t), true);

    // Verify timer expiration
    for (int i = 0; i < 4; i++)
    {
        struct timeval sleep;

        // Sleep 10 ms
        sleep.tv_sec = 0;
        sleep.tv_usec = 10 * 1000;
        EXPECT_EQ(select(0, nullptr, nullptr, nullptr, &sleep), 0);

        // Check timer expiration
        mgr_.timer_check_exp();

        if (i < 3)
        {
            // Timer shouldn't have expired but this
            // isn't real-time system, we can't assure it
        }
        else
        {
            // Timer should have expired
            ASSERT_EQ(block_.zozo_l_asticot_.size(), 1lu);
            EXPECT_EQ(block_.zozo_l_asticot_[0], std::string(arg));
        }
    }
}

//
// @brief Timer expiration order
//
TEST_F(tu_manager_tm, order)
{
    struct timer t_0;
    struct timer t_1;
    struct timer t_2;
    struct timespec time_start;
    struct timespec time_cur;
    char arg[3][8] = {"timer0", "timer1", "timer2"};

    // Get system time
    ASSERT_NE(clock_gettime(CLOCK_REALTIME, &time_start), -1);

    // Register three timers:
    //   - 40ms
    //   - 80ms
    //   - 120ms
    // Insert them in the wrong order
    t_0.bk = &block_;
    t_0.tid = 0;
    t_0.arg = arg[0];
    t_0.time.tv_sec = 0;
    t_0.time.tv_nsec = 40 * 1000 * 1000;
    t_1.bk = &block_;
    t_1.tid = 1;
    t_1.arg = arg[1];
    t_1.time.tv_sec = 0;
    t_1.time.tv_nsec = 80 * 1000 * 1000;
    t_2.bk = &block_;
    t_2.tid = 2;
    t_2.arg = arg[2];
    t_2.time.tv_sec = 0;
    t_2.time.tv_nsec = 120 * 1000 * 1000;
    EXPECT_EQ(mgr_.timer_add(t_2), true);
    EXPECT_EQ(mgr_.timer_add(t_0), true);
    EXPECT_EQ(mgr_.timer_add(t_1), true);

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
        mgr_.timer_check_exp();

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
            ASSERT_EQ(block_.zozo_l_asticot_.size(), 3lu);
            EXPECT_EQ(block_.zozo_l_asticot_[0], std::string(arg[0]));
            EXPECT_EQ(block_.zozo_l_asticot_[1], std::string(arg[1]));
            EXPECT_EQ(block_.zozo_l_asticot_[2], std::string(arg[2]));
        }
    }
}

//
// @brief Timer identification
//
TEST_F(tu_manager_tm, id)
{
    struct timer t;
    char arg[8] = "world";

    // Register two timers with the same ID:
    //   - 2000ms
    //   - 30ms
    t.bk = &block_;
    t.tid = 0;
    t.arg = arg;
    t.time.tv_sec = 0;
    t.time.tv_nsec = 2 * 1000 * 1000 * 1000;
    EXPECT_EQ(mgr_.timer_add(t), true);
    t.time.tv_sec = 0;
    t.time.tv_nsec = 30 * 1000 * 1000;
    EXPECT_EQ(mgr_.timer_add(t), true);

    // Verify only the 30ms is kept
    for (int i = 0; i < 4; i++)
    {
        struct timeval sleep;

        // Sleep 10 ms
        sleep.tv_sec = 0;
        sleep.tv_usec = 10 * 1000;
        EXPECT_EQ(select(0, nullptr, nullptr, nullptr, &sleep), 0);

        // Check timer expiration
        mgr_.timer_check_exp();

        if (i < 2)
        {
            // Timer shouldn't have expired
        }
        else
        {
            // Timer should expire
            ASSERT_EQ(block_.zozo_l_asticot_.size(), 1lu);
            EXPECT_EQ(block_.zozo_l_asticot_[0], std::string(arg));
        }
    }
}
