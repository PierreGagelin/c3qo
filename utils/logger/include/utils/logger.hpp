#ifndef C3QO_LOGGER_HPP
#define C3QO_LOGGER_HPP

// Project headers
#include "utils/include.hpp"

// Open and close connection to syslog
#define LOGGER_OPEN(name) openlog(name, 0, 0);
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
};
const char *get_logger_level(enum logger_level l);

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

#ifndef LOGGER_DISABLE

// Current level of log. Only log with lower level will be displayed
extern enum logger_level logger_level;

// Format a log entry with function name, line and level
#define LOGGER_EMERG(msg, ...)                                                           \
    if (logger_level >= LOGGER_LEVEL_EMERG)                                              \
    {                                                                                    \
        syslog(LOG_EMERG, "[EMERG] " msg " (%s:%d)", ##__VA_ARGS__, __func__, __LINE__); \
        printf("[EMERG] " msg " (%s:%d)\n", ##__VA_ARGS__, __func__, __LINE__);          \
    }
#define LOGGER_ALERT(msg, ...)                                                           \
    if (logger_level >= LOGGER_LEVEL_ALERT)                                              \
    {                                                                                    \
        syslog(LOG_ALERT, "[ALERT] " msg " (%s:%d)", ##__VA_ARGS__, __func__, __LINE__); \
        printf("[ALERT] " msg " (%s:%d)\n", ##__VA_ARGS__, __func__, __LINE__);          \
    }
#define LOGGER_CRIT(msg, ...)                                                          \
    if (logger_level >= LOGGER_LEVEL_CRIT)                                             \
    {                                                                                  \
        syslog(LOG_CRIT, "[CRIT] " msg " (%s:%d)", ##__VA_ARGS__, __func__, __LINE__); \
        printf("[CRIT] " msg " (%s:%d)\n", ##__VA_ARGS__, __func__, __LINE__);         \
    }
#define LOGGER_ERR(msg, ...)                                                         \
    if (logger_level >= LOGGER_LEVEL_ERR)                                            \
    {                                                                                \
        syslog(LOG_ERR, "[ERR] " msg " (%s:%d)", ##__VA_ARGS__, __func__, __LINE__); \
        printf("[ERR] " msg " (%s:%d)\n", ##__VA_ARGS__, __func__, __LINE__);        \
    }
#define LOGGER_WARNING(msg, ...)                                                             \
    if (logger_level >= LOGGER_LEVEL_WARNING)                                                \
    {                                                                                        \
        syslog(LOG_WARNING, "[WARNING] " msg " (%s:%d)", ##__VA_ARGS__, __func__, __LINE__); \
        printf("[WARNING] " msg " (%s:%d)\n", ##__VA_ARGS__, __func__, __LINE__);            \
    }
#define LOGGER_NOTICE(msg, ...)                                                            \
    if (logger_level >= LOGGER_LEVEL_NOTICE)                                               \
    {                                                                                      \
        syslog(LOG_NOTICE, "[NOTICE] " msg " (%s:%d)", ##__VA_ARGS__, __func__, __LINE__); \
        printf("[NOTICE] " msg " (%s:%d)\n", ##__VA_ARGS__, __func__, __LINE__);           \
    }
#define LOGGER_INFO(msg, ...)                                                          \
    if (logger_level >= LOGGER_LEVEL_INFO)                                             \
    {                                                                                  \
        syslog(LOG_INFO, "[INFO] " msg " (%s:%d)", ##__VA_ARGS__, __func__, __LINE__); \
        printf("[INFO] " msg " (%s:%d)\n", ##__VA_ARGS__, __func__, __LINE__);         \
    }
#define LOGGER_DEBUG(msg, ...)                                                           \
    if (logger_level >= LOGGER_LEVEL_DEBUG)                                              \
    {                                                                                    \
        syslog(LOG_DEBUG, "[DEBUG] " msg " (%s:%d)", ##__VA_ARGS__, __func__, __LINE__); \
        printf("[DEBUG] " msg " (%s:%d)\n", ##__VA_ARGS__, __func__, __LINE__);          \
    }

#else

//
// @brief Trick:
//          - removes call to log facility
//          - free space used by msg as it's useless (=8KB at the moment)
//          - does not trigger compilation -Werror (mainly unused variables)
//

#define LOGGER_EMERG(msg, ...)                                            \
    (void)std::tuple_size<decltype(std::make_tuple(__VA_ARGS__))>::value; \
    (void)msg;
#define LOGGER_ALERT(msg, ...)                                            \
    (void)std::tuple_size<decltype(std::make_tuple(__VA_ARGS__))>::value; \
    (void)msg;
#define LOGGER_CRIT(msg, ...)                                             \
    (void)std::tuple_size<decltype(std::make_tuple(__VA_ARGS__))>::value; \
    (void)msg;
#define LOGGER_ERR(msg, ...)                                              \
    (void)std::tuple_size<decltype(std::make_tuple(__VA_ARGS__))>::value; \
    (void)msg;
#define LOGGER_WARNING(msg, ...)                                          \
    (void)std::tuple_size<decltype(std::make_tuple(__VA_ARGS__))>::value; \
    (void)msg;
#define LOGGER_INFO(msg, ...)                                             \
    (void)std::tuple_size<decltype(std::make_tuple(__VA_ARGS__))>::value; \
    (void)msg;
#define LOGGER_DEBUG(msg, ...)                                            \
    (void)std::tuple_size<decltype(std::make_tuple(__VA_ARGS__))>::value; \
    (void)msg;

#endif

#endif // C3QO_LOGGER_HPP
