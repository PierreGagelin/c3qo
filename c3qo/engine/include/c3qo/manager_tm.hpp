#ifndef C3QO_MANAGER_TM_H
#define C3QO_MANAGER_TM_H


#include <stdbool.h> // bool
#include <time.h>    // timer_t, itimerspec


bool manager_tm_init();
void manager_tm_clean();
void manager_tm_set(timer_t tid, const struct itimerspec *its);
void manager_tm_create(timer_t *tid, void *arg, void (*callback)(void *arg));


#endif // C3QO_MANAGER_TM_H


