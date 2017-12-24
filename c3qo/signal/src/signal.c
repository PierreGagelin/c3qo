/**
 * @brief This file aims to give an API to register signal handlers
 */

#include "c3qo/signal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Registers a callback for a signal
 *
 * @param func : Callback to be called upon signal
 * @param sig  : Signal to register
 *
 * NOTE: this is not sufficient if many blocks register on a same signal
 */
void c3qo_register_fd_handler(int sig,
                void (*func)(int, siginfo_t *, void *))
{
        struct sigaction fd_callback;
        int ret;

        memset(&fd_callback, 0, sizeof(fd_callback));
        fd_callback.sa_flags     = SA_SIGINFO;
        fd_callback.sa_sigaction = func;

        ret = sigaction(sig, &fd_callback, NULL);
        if (ret != 0)
        {
                fprintf(stderr, "ERROR while registering signal handler");
                exit(EXIT_FAILURE);
        }
}

