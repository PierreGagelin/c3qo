/**
 * @brief Test file
 */


#include <unistd.h> // sleep
#include <string.h> // memcmp, strlen, strncpy
#include <stdlib.h> // exit

#include "c3qo/manager_tm.hpp" // manager_tm_init


static char zozo_l_asticot[8] = "hello";
static void tm_callback(void *arg)
{
        /* We need strlen + 1 bytes to write it */
        if (strlen((char *) arg) >= sizeof(zozo_l_asticot))
        {
                exit(1);
        }

        strncpy(zozo_l_asticot, (char *) arg, sizeof(zozo_l_asticot));
}

/**
 * @brief Test the timer manager
 */
int main(int argc, char **argv)
{
        timer_t           tid;
        struct itimerspec its;
        char              arg[8] = "world";

        (void) argc;
        (void) argv;

        manager_tm_init();

        /* Register a timer */
        manager_tm_create(&tid, arg, &tm_callback);

        /* Set the timer to trigger each 1ms */
        its.it_value.tv_sec  = 0;
        its.it_value.tv_nsec = 1000000;

        its.it_interval.tv_sec  = its.it_value.tv_sec;
        its.it_interval.tv_nsec = its.it_value.tv_nsec;

        manager_tm_set(tid, &its);

        /* Should be awaken by timer */
        sleep(1);

        /* Verify callback was called */
        if (memcmp(zozo_l_asticot, arg, strlen(arg) + 1) != 0)
        {
                exit(1);
        }
        
        manager_tm_clean();
}


