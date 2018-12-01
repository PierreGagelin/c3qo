#ifndef LOGGER_HPP
#define LOGGER_HPP

// Project headers
#include "utils/include.hpp"

// C headers
extern "C"
{
#include <syslog.h>
}

// Open and close connection to syslog
#define LOGGER_OPEN(name) openlog(name, 0, 0)
#define LOGGER_CLOSE() closelog()

// Differents levels of log
enum logger_level
{
    LOGGER_LEVEL_NONE = 0,
    LOGGER_LEVEL_EMERG = 1,
    LOGGER_LEVEL_ALERT = 2,
    LOGGER_LEVEL_CRIT = 3,
    LOGGER_LEVEL_ERR = 4,
    LOGGER_LEVEL_WARNING = 5,
    LOGGER_LEVEL_NOTICE = 6,
    LOGGER_LEVEL_INFO = 7,
    LOGGER_LEVEL_DEBUG = 8,
};
const char *get_logger_level(enum logger_level l);

// Setting logger level
void logger_set_level(enum logger_level l);

//
// MAN SYSLOG :
// - LOG_EMERG   : system is unusable
// - LOG_ALERT   : action must be taken immediately
// - LOG_CRIT    : critical conditions
// - LOG_ERR     : error conditions
// - LOG_WARNING : warning conditions
// - LOG_NOTICE  : normal, but significant, condition
// - LOG_INFO    : informational message
// - LOG_DEBUG   : debug-level message
//

#ifdef C3QO_LOG

#define LOGGER_TRACE(level, msg, ...)                 \
    syslog(level, LOGGER_TAG " " msg, ##__VA_ARGS__); \
    printf("[%s]" LOGGER_TAG " " msg "\n", get_logger_level(static_cast<enum logger_level>(level + 1)), ##__VA_ARGS__);

#else

// Code will be removed if unused and without a warning
#define LOGGER_TRACE(level, msg, ...)                     \
    if (false == true)                                    \
    {                                                     \
        syslog(level, LOGGER_TAG " " msg, ##__VA_ARGS__); \
    }

#endif // C3QO_LOG

// Current level of log. Only log with lower level will be displayed
extern enum logger_level logger_level;

#ifndef LOGGER_TAG
#define LOGGER_TAG ""
#endif

// Format a log entry with function name, line and level
#define LOGGER_EMERG(msg, ...)                                                      \
    if (logger_level >= LOGGER_LEVEL_EMERG)                                         \
    {                                                                               \
        LOGGER_TRACE(LOG_EMERG, msg " (%s:%d)", ##__VA_ARGS__, __func__, __LINE__); \
    }
#define LOGGER_ALERT(msg, ...)                                                      \
    if (logger_level >= LOGGER_LEVEL_ALERT)                                         \
    {                                                                               \
        LOGGER_TRACE(LOG_ALERT, msg " (%s:%d)", ##__VA_ARGS__, __func__, __LINE__); \
    }
#define LOGGER_CRIT(msg, ...)                                                      \
    if (logger_level >= LOGGER_LEVEL_CRIT)                                         \
    {                                                                              \
        LOGGER_TRACE(LOG_CRIT, msg " (%s:%d)", ##__VA_ARGS__, __func__, __LINE__); \
    }
#define LOGGER_ERR(msg, ...)                                                      \
    if (logger_level >= LOGGER_LEVEL_ERR)                                         \
    {                                                                             \
        LOGGER_TRACE(LOG_ERR, msg " (%s:%d)", ##__VA_ARGS__, __func__, __LINE__); \
    }
#define LOGGER_WARNING(msg, ...)                                                      \
    if (logger_level >= LOGGER_LEVEL_WARNING)                                         \
    {                                                                                 \
        LOGGER_TRACE(LOG_WARNING, msg " (%s:%d)", ##__VA_ARGS__, __func__, __LINE__); \
    }
#define LOGGER_NOTICE(msg, ...)                                                      \
    if (logger_level >= LOGGER_LEVEL_NOTICE)                                         \
    {                                                                                \
        LOGGER_TRACE(LOG_NOTICE, msg " (%s:%d)", ##__VA_ARGS__, __func__, __LINE__); \
    }
#define LOGGER_INFO(msg, ...)                                                      \
    if (logger_level >= LOGGER_LEVEL_INFO)                                         \
    {                                                                              \
        LOGGER_TRACE(LOG_INFO, msg " (%s:%d)", ##__VA_ARGS__, __func__, __LINE__); \
    }
#define LOGGER_DEBUG(msg, ...)                                                      \
    if (logger_level >= LOGGER_LEVEL_DEBUG)                                         \
    {                                                                               \
        LOGGER_TRACE(LOG_DEBUG, msg " (%s:%d)", ##__VA_ARGS__, __func__, __LINE__); \
    }

#define ASSERT(condition)                                           \
    do                                                              \
    {                                                               \
        if ((condition) == false)                                   \
        {                                                           \
            LOGGER_CRIT("Failed to assert condition: " #condition); \
            exit(1);                                                \
        }                                                           \
    } while (false)

#endif // LOGGER_HPP
