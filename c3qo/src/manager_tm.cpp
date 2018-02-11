//
// @brief Synchronous timer management
//          - add timer with callback and argument
//          - remove timer
//        Is meant to be statically used
//

// C++ library headers
#include <cstdlib> // NULL, abs

// Project headers
#include "c3qo/manager_tm.hpp"
#include "utils/logger.hpp"

// One static instance of the timer manager
class manager_tm m_tm;

//
// @brief Add or replace an entry in the timer list
//
bool manager_tm::add(struct timer &tm)
{
    struct timeval t;

    // Verify user input
    if (tm.callback == NULL)
    {
        LOGGER_WARNING("Cannot add timer without a callback [tid=%d ; sec=%ld ; usec=%ld]", tm.tid, (long)tm.time.tv_sec, (long)tm.time.tv_usec);
        return false;
    }

    // Convert relative time to absolute time
    if (gettimeofday(&t, NULL) == -1)
    {
        LOGGER_ERR("Failed call to gettimeofday for timer [tid=%d ; sec=%ld ; usec=%ld]", tm.tid, (long)tm.time.tv_sec, (long)tm.time.tv_usec);
        return false;
    }
    tm.time.tv_sec += t.tv_sec;
    tm.time.tv_usec += t.tv_usec;

    // Wrap microseconds value to be both in range and positive
    tm.time.tv_sec += tm.time.tv_usec / USEC_MAX;
    tm.time.tv_usec = tm.time.tv_usec % USEC_MAX;

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
    struct timeval time;

    if (gettimeofday(&time, NULL) == -1)
    {
        LOGGER_WARNING("Couldn't retrieve time of day, will try it next time [sec=%ld ; usec=%ld]", (long)time.tv_sec, (long)time.tv_usec);
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

// Operators for struct timeval
bool operator==(const struct timeval &a, const struct timeval &b)
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
bool operator<(const struct timeval &a, const struct timeval &b)
{
    if (a.tv_sec < b.tv_sec)
    {
        return true;
    }
    else if (a.tv_sec == b.tv_sec)
    {
        return (a.tv_usec < b.tv_usec);
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
