//
// @brief Synchronous timer management
//          - add timer with callback and argument
//          - remove timer
//        Is meant to be statically used
//

// Project headers
#include "c3qo/manager_tm.hpp"

//
// @brief Add or replace an entry in the timer list
//
bool manager_tm::add(struct timer &tm)
{
    struct timespec t;

    // Verify user input
    if (tm.callback == NULL)
    {
        LOGGER_WARNING("Cannot add timer: NULL callback [tid=%d ; sec=%ld ; nsec=%ld]", tm.tid, (long)tm.time.tv_sec, (long)tm.time.tv_nsec);
        return false;
    }

    // Convert relative time to absolute time
    if (clock_gettime(CLOCK_REALTIME, &t) == -1)
    {
        LOGGER_ERR("Failed to add timer: clock_gettime failed [tid=%d ; sec=%ld ; nsec=%ld]", tm.tid, (long)tm.time.tv_sec, (long)tm.time.tv_nsec);
        return false;
    }
    tm.time.tv_sec += t.tv_sec;
    tm.time.tv_nsec += t.tv_nsec;

    // Wrap microseconds value to be both in range and positive
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
void manager_tm::del(struct timer &tm)
{
    tm_list_.remove(tm);
}

//
// @brief Check timers expiration and execute callbacks
//
void manager_tm::check_exp()
{
    struct timer timer;
    struct timespec time;

    if (clock_gettime(CLOCK_REALTIME, &time) == -1)
    {
        LOGGER_ERR("Failed to check expiration: clock_gettime failed [sec=%ld ; nsec=%ld]", (long)time.tv_sec, (long)time.tv_nsec);
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
            timer.callback(timer.arg);
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
void manager_tm::clear()
{
    tm_list_.clear();
}

// Operators for struct timespec
bool operator==(const struct timespec &a, const struct timespec &b)
{
    if ((a.tv_sec == b.tv_sec) && (a.tv_sec == b.tv_sec))
    {
        return true;
    }
    else
    {
        return false;
    }
}
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
