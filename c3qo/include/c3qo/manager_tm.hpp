#ifndef C3QO_MANAGER_TM_HPP
#define C3QO_MANAGER_TM_HPP

// C++ library headers
#include <forward_list> // forward_list container

// System library headers
extern "C" {
#include <sys/time.h> // gettimeofday, timeval
}

#define USEC_MAX 1000000 // Maximum number of usec + 1

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

bool operator==(const struct timer &a, const struct timer &b);
bool operator<(const struct timer &a, const struct timer &b);

bool operator==(const struct timeval &a, const struct timeval &b);
bool operator<(const struct timeval &a, const struct timeval &b);

class manager_tm
{
  protected:
    // Singly-linked list of timers sorted by expiration
    std::forward_list<struct timer> tm_list_;

  public:
    bool add(struct timer &tm);
    void del(struct timer &tm);

  public:
    void check_exp();

  public:
    void clear();
};

#endif // C3QO_MANAGER_TM_HPP
