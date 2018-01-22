

#include <signal.h>  /* timer_create, sigevent, sigaction */
#include <string.h>  /* memset, memcpy */

#include "c3qo/logger.h"     /* LOGGER */
#include "c3qo/manager_tm.h" /* bool, timer_t, itimerspec */


/* Signal and clock used by timers */
#define MANAGER_TM_SIGNAL SIGRTMIN
#define MANAGER_TM_CLOCK  CLOCK_REALTIME

/* Maximum number of timer handled at a time */
#define MANAGER_TM_MAX 256


struct manager_tm_entry
{
        timer_t  tid;                  /* Timer ID */
        void     *arg;                 /* Argument to give to the callback */
        void    (*callback)(void *arg); /* Function to call when timer expires */
};
struct manager_tm_entry tm_list[MANAGER_TM_MAX];


/**
 * @brief Find a timer ID in the list of timer entries
 *
 * @return index in the list on success, -1 on failure
 */
static inline int manager_tm_find(timer_t tid)
{
        int i;

        for (i = 0; i < MANAGER_TM_MAX; i++)
        {
                if (tm_list[i].tid != tid)
                {
                        continue;
                }

                /* Found */
                return i;
        }

        /* Not found */
        return -1;
}


/**
 * @brief Add a new entry in the timer list
 */
static bool manager_tm_add(struct manager_tm_entry *entry)
{
        int idx;

        /* Find an empty entry */
        idx = manager_tm_find(NULL);
        if (idx == -1)
        {
                LOGGER_ERR("Failed to find room to add timer [timer_id=%p ; arg=%p ; callback=%p]", entry->tid, entry->arg, entry->callback);
                return false;
        }

        /* Register the entry */
        memcpy(&tm_list[idx], entry, sizeof(tm_list[idx]));

        return true;
}


/**
 * @brief Delete a timer entry
 */
static void manager_tm_del(timer_t tid)
{
        int idx;

        if (tid == NULL)
        {
                /* Nothing to do */
                return;
        }

        /* Find the entry */
        idx = manager_tm_find(tid);
        if (idx == -1)
        {
                /* Nothing to do */
                return;
        }

        /* Delete the timer */
        if (timer_delete(tm_list[idx].tid) == -1)
        {
                LOGGER_CRIT("Failed call to timer_delete [timer_id=%p]", tm_list[idx].tid);
        }
        
        /* Delete the entry from the list */
        memset(&tm_list[idx], 0, sizeof(tm_list[idx]));
}


/**
 * @brief Retrieve registered callback from signal
 */
static void manager_tm_handler(int sig, siginfo_t *si, void *uc)
{
        timer_t *tid;
        int     idx;

        (void) uc; /* Unused */

        /* Verify the signal */
        if (sig != MANAGER_TM_SIGNAL)
        {
                /* Nothing to do */
                return;
        }

        /* Find the timer entry */
        tid = (timer_t *) si->si_value.sival_ptr;
        idx = manager_tm_find(*tid);
        if (idx == -1)
        {
                LOGGER_ERR("Failed to find context associated to expired timer [timer_id=%p]", *tid);
        }

        /* Call the callback */
        tm_list[idx].callback(tm_list[idx].arg);
}


/**
 * @brief Create a timer for a callback
 *
 * @param tid      : Timer ID
 * @param arg      : Argument to give to the callback
 * @param callback : Function to call on timer signal
 */
void manager_tm_create(timer_t *tid, void *arg, void (*callback)(void *arg))
{
        struct manager_tm_entry timer;
        struct sigevent         sev;

        /* Verify user input */
        if (tid == NULL)
        {
                LOGGER_WARNING("Cannot add timer with this ID [timer_id=%p]", tid);
                return;
        }
        if (callback == NULL)
        {
                LOGGER_WARNING("Cannot add timer with this callback [callback=%p]", callback);
                return;
        }

        /* Verify timer does not exist */
        if (manager_tm_find(tid) != -1)
        {
                LOGGER_WARNING("Cannot add timer with an already used ID [timer_id=%p]", tid);
                return;
        }

        /* Get an ID for the timer */
        sev.sigev_notify          = SIGEV_SIGNAL;
        sev.sigev_signo           = MANAGER_TM_SIGNAL;
        sev.sigev_value.sival_ptr = tid;
        if (timer_create(MANAGER_TM_CLOCK, &sev, tid) == -1)
        {
                LOGGER_ERR("Failed call to timer_create");
                return;
        }

        /* Register an entry */
        timer.tid      = *tid;
        timer.arg      = arg;
        timer.callback = callback;
        manager_tm_add(&timer);
}


/**
 * @brief Set a time interval for the timer
 *
 * @param its : time specification
 */
void manager_tm_set(timer_t tid, const struct itimerspec *its)
{
        struct timespec zero;
        int             idx;

        /* Verify user input */
        if (tid == NULL)
        {
                /* Nothing to do */
                return;
        }

        /* Find the timer */
        idx = manager_tm_find(tid);
        if (idx == -1)
        {
                /* Nothing to do */
                return;
        }

        /* Verify if it's an unset */
        memset(&zero, 0, sizeof(zero));
        if(memcmp(&its->it_value, &zero, sizeof(zero)) == 0)
        {
                manager_tm_del(tm_list[idx].tid);
                return;
        }

        /* Set a time specificaion */
        if (timer_settime(tm_list[idx].tid, 0, its, NULL) == -1)
        {
                LOGGER_ERR("Failed call to timer_settime [timer_id=%p]", tm_list[idx].tid);
                return;
        }
}


/**
 * @brief Initialize the timer management
 */
bool manager_tm_init()
{
        struct sigaction sa;

        LOGGER_INFO("Initialize timer manager");

        /* Initialize list */
        memset(tm_list, 0, sizeof(tm_list));

        /* Register a callback to deal with a signal that will be triggered by timers */
        sa.sa_flags     = SA_SIGINFO;
        sa.sa_sigaction = manager_tm_handler;
        sigemptyset(&sa.sa_mask);
        if (sigaction(MANAGER_TM_SIGNAL, &sa, NULL) == -1)
        {
                LOGGER_ERR("Failed call to sigaction");
                return false;
        }

        return true;
}


void manager_tm_clean()
{
        int i;

        for (i = 0; i < MANAGER_TM_MAX; i++)
        {
                manager_tm_del(tm_list[i].tid);
        }
}


