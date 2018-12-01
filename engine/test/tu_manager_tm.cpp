//
// @brief Test file for the block manager
//

#define LOGGER_TAG "[TU.engine.timer]"

// Project headers
#include "engine/tu.hpp"

struct block_timer : block
{
    std::vector<std::string> zozo_l_asticot_;

    explicit block_timer(struct manager *mgr) : block(mgr) {}

    virtual void on_timer_(struct timer &tm) override final
    {
        zozo_l_asticot_.push_back(std::string(static_cast<const char *>(tm.arg)));
    }
};

struct manager mgr_;
struct block_timer block_(&mgr_);

//
// @brief Timer expiration
//
static void tu_manager_tm_expiration()
{
    struct timer t;
    char dummy[8] = "dummy";

    // Register a 1 ms timer
    t.tid = 0;
    t.bk = &block_;
    t.arg = dummy;
    t.time.tv_sec = 0;
    t.time.tv_nsec = 1 * 1000 * 1000;
    ASSERT(mgr_.timer_add(t) == true);

    // Verify timer expiration
    // We can't verify it doesn't expire earlier
    // because it's not a real-time system
    usleep(1 * 1000);
    mgr_.timer_check_exp();
    ASSERT(block_.zozo_l_asticot_.size() == 1u);
    block_.zozo_l_asticot_.clear();
}

//
// @brief Timer expiration order
//
static void tu_manager_tm_order()
{
    struct timer t_0;
    struct timer t_1;
    struct timer t_2;
    char arg[3][8] = {"timer0", "timer1", "timer2"};

    // Register three timers:
    //   - 10ms
    //   - 20ms
    //   - 30ms
    // Insert them in the wrong order
    //
    // Hope it takes less than 10ms to register
    // (can be wrong with valgrind)
    t_0.bk = &block_;
    t_0.tid = 0;
    t_0.arg = arg[0];
    t_0.time.tv_sec = 0;
    t_0.time.tv_nsec = 10 * 1000 * 1000;
    t_1.bk = &block_;
    t_1.tid = 1;
    t_1.arg = arg[1];
    t_1.time.tv_sec = 0;
    t_1.time.tv_nsec = 20 * 1000 * 1000;
    t_2.bk = &block_;
    t_2.tid = 2;
    t_2.arg = arg[2];
    t_2.time.tv_sec = 0;
    t_2.time.tv_nsec = 30 * 1000 * 1000;
    ASSERT(mgr_.timer_add(t_2) == true);
    ASSERT(mgr_.timer_add(t_0) == true);
    ASSERT(mgr_.timer_add(t_1) == true);

    // Verify the order of expiration
    usleep(30 * 1000);
    mgr_.timer_check_exp();
    ASSERT(block_.zozo_l_asticot_.size() == 3lu);
    ASSERT(block_.zozo_l_asticot_[0] == std::string(arg[0]));
    ASSERT(block_.zozo_l_asticot_[1] == std::string(arg[1]));
    ASSERT(block_.zozo_l_asticot_[2] == std::string(arg[2]));
    block_.zozo_l_asticot_.clear();

    // Check struct timer operator<
    {
        struct timer a;
        struct timer b;

        // Compare sec
        a.time.tv_sec = 0;
        a.time.tv_nsec = 0;
        b.time.tv_sec = 1;
        b.time.tv_nsec = 0;
        ASSERT(operator<(a, b) == true);
        ASSERT(operator<(b, a) == false);

        // Compare sec then nsec
        a.time.tv_sec = 1;
        a.time.tv_nsec = 0;
        b.time.tv_sec = 1;
        b.time.tv_nsec = 1;
        ASSERT(operator<(a, b) == true);
        ASSERT(operator<(b, a) == false);

        // Compare sec then nsec
        a.time.tv_sec = 1;
        a.time.tv_nsec = 0;
        b.time.tv_sec = 1;
        b.time.tv_nsec = 0;
        ASSERT(operator<(a, b) == false);
        ASSERT(operator<(b, a) == false);
    }
}

//
// @brief Timer identification
//
static void tu_manager_tm_id()
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
    ASSERT(mgr_.timer_add(t) == true);
    t.arg = arg;
    ASSERT(mgr_.timer_add(t) == true);

    // Verify only the 1ms is kept and expired
    usleep(1 * 1000);
    mgr_.timer_check_exp();
    ASSERT(block_.zozo_l_asticot_.size() > 0u);
    ASSERT(block_.zozo_l_asticot_.size() == 1u);
    ASSERT(block_.zozo_l_asticot_[0] == std::string(arg));

    block_.zozo_l_asticot_.clear();
}

//
// @brief Timer removal
//
static void tu_manager_tm_del()
{
    struct timer t;
    char dummy[8] = "dummy";

    // Register a timer
    t.bk = &block_;
    t.tid = 0;
    t.arg = dummy;
    t.time.tv_sec = 0;
    t.time.tv_nsec = 1 * 1000 * 1000;
    ASSERT(mgr_.timer_add(t) == true);

    // Remove the timer
    mgr_.timer_del(t);

    // Verify that it does not expire
    usleep(1 * 1000);
    mgr_.timer_check_exp();
    ASSERT(block_.zozo_l_asticot_.size() == 0u);

    mgr_.timer_clear();
}

//
// @brief Timer error conditions
//
static void tu_manager_tm_error()
{
    // Hide logs as errors are normal
    logger_set_level(LOGGER_LEVEL_ERR);

    struct timer t;
    t.bk = nullptr;
    ASSERT(mgr_.timer_add(t) == false);

    logger_set_level(LOGGER_LEVEL_DEBUG);
}

int main(int, char **)
{
    LOGGER_OPEN("tu_manager_tm");
    logger_set_level(LOGGER_LEVEL_DEBUG);

    tu_manager_tm_del();
    tu_manager_tm_error();
    tu_manager_tm_expiration();
    tu_manager_tm_id();
    tu_manager_tm_order();

    LOGGER_CLOSE();
    return 0;
}
