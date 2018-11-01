//
// @brief Test file for the block manager
//

#define LOGGER_TAG "[TU.engine.timer]"

// Project headers
#include "c3qo/tu.hpp"

struct block_timer : block
{
    std::vector<std::string> zozo_l_asticot_;

    explicit block_timer(struct manager *mgr) : block(mgr) {}

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
    char dummy[8] = "dummy";

    // Register a 1 ms timer
    t.tid = 0;
    t.bk = &block_;
    t.arg = dummy;
    t.time.tv_sec = 0;
    t.time.tv_nsec = 1 * 1000 * 1000;
    EXPECT_EQ(mgr_.timer_add(t), true);

    // Verify timer expiration
    // We can't verify it doesn't expire earlier
    // because it's not a real-time system
    usleep(1 * 1000);
    mgr_.timer_check_exp();
    EXPECT_EQ(block_.zozo_l_asticot_.size(), 1u);
}

//
// @brief Timer expiration order
//
TEST_F(tu_manager_tm, order)
{
    struct timer t_0;
    struct timer t_1;
    struct timer t_2;
    char arg[3][8] = {"timer0", "timer1", "timer2"};

    // Register three timers:
    //   - 1ms
    //   - 2ms
    //   - 3ms
    // Insert them in the wrong order
    // Hope it takes less than 1ms to register
    t_0.bk = &block_;
    t_0.tid = 0;
    t_0.arg = arg[0];
    t_0.time.tv_sec = 0;
    t_0.time.tv_nsec = 1 * 1000 * 1000;
    t_1.bk = &block_;
    t_1.tid = 1;
    t_1.arg = arg[1];
    t_1.time.tv_sec = 0;
    t_1.time.tv_nsec = 2 * 1000 * 1000;
    t_2.bk = &block_;
    t_2.tid = 2;
    t_2.arg = arg[2];
    t_2.time.tv_sec = 0;
    t_2.time.tv_nsec = 3 * 1000 * 1000;
    EXPECT_EQ(mgr_.timer_add(t_2), true);
    EXPECT_EQ(mgr_.timer_add(t_0), true);
    EXPECT_EQ(mgr_.timer_add(t_1), true);

    // Verify the order of expiration
    usleep(3 * 1000);
    mgr_.timer_check_exp();
    ASSERT_EQ(block_.zozo_l_asticot_.size(), 3lu);
    EXPECT_EQ(block_.zozo_l_asticot_[0], std::string(arg[0]));
    EXPECT_EQ(block_.zozo_l_asticot_[1], std::string(arg[1]));
    EXPECT_EQ(block_.zozo_l_asticot_[2], std::string(arg[2]));

    // Check struct timer operator<
    {
        struct timer a;
        struct timer b;

        // Compare sec
        a.time.tv_sec = 0;
        a.time.tv_nsec = 0;
        b.time.tv_sec = 1;
        b.time.tv_nsec = 0;
        EXPECT_TRUE(a < b);
        EXPECT_FALSE(b < a);

        // Compare sec then nsec
        a.time.tv_sec = 1;
        a.time.tv_nsec = 0;
        b.time.tv_sec = 1;
        b.time.tv_nsec = 1;
        EXPECT_TRUE(a < b);
        EXPECT_FALSE(b < a);

        // Compare sec then nsec
        a.time.tv_sec = 1;
        a.time.tv_nsec = 0;
        b.time.tv_sec = 1;
        b.time.tv_nsec = 0;
        EXPECT_FALSE(a < b);
        EXPECT_FALSE(b < a);
    }
}

//
// @brief Timer identification
//
TEST_F(tu_manager_tm, id)
{
    struct timer t;
    char arg[] = "expected";
    char dummy[] = "dummy";

    // Register two timers with the same ID:
    t.bk = &block_;
    t.tid = 0;
    t.time.tv_sec = 0;
    t.time.tv_nsec = 1 * 1000 * 1000;

    t.arg = dummy;
    EXPECT_EQ(mgr_.timer_add(t), true);
    t.arg = arg;
    EXPECT_EQ(mgr_.timer_add(t), true);

    // Verify only the 1ms is kept and expired
    usleep(1 * 1000);
    mgr_.timer_check_exp();
    ASSERT_GT(block_.zozo_l_asticot_.size(), 0u);
    EXPECT_EQ(block_.zozo_l_asticot_.size(), 1u);
    EXPECT_EQ(block_.zozo_l_asticot_[0], std::string(arg));
}

//
// @brief Timer removal
//
TEST_F(tu_manager_tm, del)
{
    struct timer t;
    char dummy[8] = "dummy";

    // Register several different timers
    t.bk = &block_;
    t.tid = 0;
    t.arg = dummy;
    t.time.tv_sec = 0;
    t.time.tv_nsec = 1 * 1000 * 1000;
    EXPECT_EQ(mgr_.timer_add(t), true);

    // Remove the timer
    mgr_.timer_del(t);

    // Verify that it does not expire
    usleep(1 * 1000);
    mgr_.timer_check_exp();
    EXPECT_EQ(block_.zozo_l_asticot_.size(), 0u);
}

//
// @brief Timer error conditions
//
TEST_F(tu_manager_tm, error)
{
    // Hide logs as errors are normal
    logger_set_level(LOGGER_LEVEL_NONE);

    struct timer t;
    t.bk = nullptr;
    EXPECT_EQ(mgr_.timer_add(t), false);
}
