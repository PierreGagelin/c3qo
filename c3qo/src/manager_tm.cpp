//
// @brief Synchronous timer management
//          - add timer with callback and argument
//          - remove timer
//        Is meant to be statically used
//

#define LOGGER_TAG "[engine.timer]"

// Project headers
#include "c3qo/manager.hpp"

//
// @brief Add or replace an entry in the timer list
//
// It takes a copy in order not to change time value in caller
//
bool manager::timer_add(struct timer tm)
{
    struct timespec t;

    // Verify user input
    if (tm.bk == nullptr)
    {
        LOGGER_ERR("Cannot add timer: nullptr block [tid=%d]", tm.tid);
        return false;
    }

    // Convert relative time to absolute time
    if (clock_gettime(CLOCK_REALTIME, &t) == -1)
    {
        LOGGER_ERR("Failed to add timer: clock_gettime failed [tid=%d]", tm.tid);
        return false;
    }
    tm.time.tv_sec += t.tv_sec;
    tm.time.tv_nsec += t.tv_nsec;

    // Wrap nanoseconds value to be both in range and positive
    tm.time.tv_sec += tm.time.tv_nsec / NSEC_MAX;
    tm.time.tv_nsec = tm.time.tv_nsec % NSEC_MAX;

    // Remove potentially existing timer and push new one
    tm_list_.remove(tm);
    tm_list_.push_front(tm);

    // Keep the list ordered by expiration date
    // XXX: could be optimized in the case of a new timer because
    //      we known that the list is already sorted
    tm_list_.sort();

    return true;
}

//
// @brief Delete a timer
//
void manager::timer_del(const struct timer &tm)
{
    tm_list_.remove(tm);
}

//
// @brief Check timers expiration and execute callbacks
//
void manager::timer_check_exp()
{
    struct timer timer;
    struct timespec time;

    if (clock_gettime(CLOCK_REALTIME, &time) == -1)
    {
        LOGGER_ERR("Failed to check expiration: clock_gettime failed");
        return;
    }

    while (true)
    {
        if (tm_list_.empty() == true)
        {
            break;
        }

        timer = tm_list_.front();
        if (timer.time < time)
        {
            // Timer has expired: remove timer and execute callback
            //   - order matters: callback could register the timer again
            tm_list_.pop_front();
            timer.bk->on_timer_(timer);
        }
        else
        {
            // Most recent timer not yet expired
            break;
        }
    }
}

//
// @brief Clear every timer
//
void manager::timer_clear()
{
    tm_list_.clear();
}

// Operators for struct timespec
bool operator<(const struct timespec &a, const struct timespec &b)
{
    if (a.tv_sec < b.tv_sec)
    {
        return true;
    }
    else if (a.tv_sec == b.tv_sec)
    {
        return (a.tv_nsec < b.tv_nsec);
    }
    else
    {
        return false;
    }
}

// Operator for struct timer (necessary for the remove method)
bool operator==(const struct timer &a, const struct timer &b)
{
    return (a.tid == b.tid);
}
// Operator for struct timer (necessary for the sort method)
bool operator<(const struct timer &a, const struct timer &b)
{
    return (a.time < b.time);
}
