#ifndef _C3QO_SIGNAL_H_
#define _C3QO_SIGNAL_H_

#include <signal.h>

void c3qo_register_fd_handler(int sig,
                void (*func)(int, siginfo_t *, void *));

#endif /* _C3QO_SIGNAL_H_ */
