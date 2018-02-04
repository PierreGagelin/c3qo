//
// @brief Synchronous timer management
//          - add timer with callback and argument
//          - remove timer
//        Is meant to be statically used
//

// C++ library headers
#include <forward_list> // forward_list container
#include <cstdlib>      // NULL

// Project headers
#include "c3qo/manager_tm.hpp"
#include "utils/logger.hpp"

#define USEC_MAX 1000000 // Maximum number of usec

namespace manager_tm
{

// Singly-linked list of timers sorted by expiration
std::forward_list<struct timer> tm_list_;

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

//
// @brief Add or replace an entry in the timer list
//
bool add(struct timer &tm)
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
    if (tm.time.tv_usec >= USEC_MAX)
    {
        // Convert usec to sec
        tm.time.tv_sec += tm.time.tv_usec / USEC_MAX;
        tm.time.tv_usec = tm.time.tv_usec % USEC_MAX;
    }

    // Remove potentially existing timer and push new one
    manager_tm::tm_list_.remove(tm);
    manager_tm::tm_list_.push_front(tm);

    // Keep the list ordered by expiration date
    // XXX: could be optimized in the case of a new timer because
    //      we known that the list is already sorted
    manager_tm::tm_list_.sort();

    return true;
}

//
// @brief Delete a timer
//
void del(struct timer &tm)
{
    manager_tm::tm_list_.remove(tm);
}

//
// @brief Check timers expiration and execute callbacks
//
void check_exp()
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
        if (manager_tm::tm_list_.empty() == true)
        {
            break;
        }

        timer = manager_tm::tm_list_.front();
        if (timer.time < time)
        {
            // Timer has expired
            timer.callback(timer.arg);
            manager_tm::tm_list_.pop_front();
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
void clear()
{
    manager_tm::tm_list_.clear();
}

} // END namespace manager_tm
