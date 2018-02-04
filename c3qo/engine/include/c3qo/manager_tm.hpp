#ifndef C3QO_MANAGER_TM_H
#define C3QO_MANAGER_TM_H

extern "C" {
#include <sys/time.h> // gettimeofday, timeval
}

namespace manager_tm
{

//
// @struct timer
//
struct timer
{
    int tid;                     // Timer ID
    struct timeval time;         // Expiration time for the timer
    void (*callback)(void *arg); // Function to call when timer expires
    void *arg;                   // Argument to give to the callback
};

void clear();
void check_exp();
void del(struct timer &tm);
bool add(struct timer &tm);

bool operator==(const struct timeval &a, const struct timeval &b);
bool operator<(const struct timeval &a, const struct timeval &b);

bool operator==(const struct timer &a, const struct timer &b);
bool operator<(const struct timer &a, const struct timer &b);

} // END namespace manager_tm

#endif // C3QO_MANAGER_TM_H
