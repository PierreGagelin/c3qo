#ifndef C3QO_LOGGER_H
#define C3QO_LOGGER_H

#include <syslog.h> // syslog

// Open and close connection to syslog
#define LOGGER_OPEN() openlog("c3qo", 0, 0);
#define LOGGER_CLOSE() closelog();

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
    LOGGER_LEVEL_MAX = 8,
};

// Current level of log. Only log with lower level will be displayed
extern enum logger_level logger_level;

// Setting logger level
void logger_set_level(enum logger_level l);

//
// MAN SYSLOG :
//   - LOG_EMERG   : system is unusable
//   - LOG_ALERT   : action must be taken immediately
//   - LOG_CRIT    : critical conditions
//   - LOG_ERR     : error conditions
//   - LOG_WARNING : warning conditions
//   - LOG_NOTICE  : normal, but significant, condition
//   - LOG_INFO    : informational message
//   - LOG_DEBUG   : debug-level message
//

// Format a log entry with function name, line and level
#define LOGGER_EMERG(msg, ...)                                                           \
    if (logger_level >= LOGGER_LEVEL_EMERG)                                              \
    {                                                                                    \
        syslog(LOG_EMERG, "[EMERG] " msg " (%s:%d)", ##__VA_ARGS__, __func__, __LINE__); \
    }
#define LOGGER_ALERT(msg, ...)                                                           \
    if (logger_level >= LOGGER_LEVEL_ALERT)                                              \
    {                                                                                    \
        syslog(LOG_ALERT, "[ALERT] " msg " (%s:%d)", ##__VA_ARGS__, __func__, __LINE__); \
    }
#define LOGGER_CRIT(msg, ...)                                                          \
    if (logger_level >= LOGGER_LEVEL_CRIT)                                             \
    {                                                                                  \
        syslog(LOG_CRIT, "[CRIT] " msg " (%s:%d)", ##__VA_ARGS__, __func__, __LINE__); \
    }
#define LOGGER_ERR(msg, ...)                                                         \
    if (logger_level >= LOGGER_LEVEL_ERR)                                            \
    {                                                                                \
        syslog(LOG_ERR, "[ERR] " msg " (%s:%d)", ##__VA_ARGS__, __func__, __LINE__); \
    }
#define LOGGER_WARNING(msg, ...)                                                             \
    if (logger_level >= LOGGER_LEVEL_WARNING)                                                \
    {                                                                                        \
        syslog(LOG_WARNING, "[WARNING] " msg " (%s:%d)", ##__VA_ARGS__, __func__, __LINE__); \
    }
#define LOGGER_NOTICE(msg, ...)                                                            \
    if (logger_level >= LOGGER_LEVEL_NOTICE)                                               \
    {                                                                                      \
        syslog(LOG_NOTICE, "[NOTICE] " msg " (%s:%d)", ##__VA_ARGS__, __func__, __LINE__); \
    }
#define LOGGER_INFO(msg, ...)                                                          \
    if (logger_level >= LOGGER_LEVEL_INFO)                                             \
    {                                                                                  \
        syslog(LOG_INFO, "[INFO] " msg " (%s:%d)", ##__VA_ARGS__, __func__, __LINE__); \
    }
#define LOGGER_DEBUG(msg, ...)                                                           \
    if (logger_level >= LOGGER_LEVEL_DEBUG)                                              \
    {                                                                                    \
        syslog(LOG_DEBUG, "[DEBUG] " msg " (%s:%d)", ##__VA_ARGS__, __func__, __LINE__); \
    }

#endif // C3QO_LOGGER_H
