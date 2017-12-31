#ifndef C3QO_SIGNAL_H
#define C3QO_SIGNAL_H


#include <signal.h>


void c3qo_register_fd_handler(int sig, void (*func)(int, siginfo_t *, void *));


#endif


