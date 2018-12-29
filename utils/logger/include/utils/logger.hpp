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

// Enable or disable logging
extern bool logger_enabled;
#define LOGGER_ENABLE() logger_enabled = true
#define LOGGER_DISABLE() logger_enabled = false

#ifdef C3QO_LOG

#define LOGGER_TRACE(level, msg, ...)                \
    if (logger_enabled == true)                      \
    {                                                \
        syslog(level, msg, ##__VA_ARGS__);           \
        printf(#level ": " msg "\n", ##__VA_ARGS__); \
    }

#else

#include <tuple>

//
// Trick to remove any trace of log
//
#define LOGGER_TRACE(level, msg, ...)                                         \
    do                                                                        \
    {                                                                         \
        (void)level;                                                          \
        (void)msg;                                                            \
        (void)std::tuple_size<decltype(std::make_tuple(__VA_ARGS__))>::value; \
    } while (false)

#endif // C3QO_LOG

//
// Some different level of traces mapped to syslog levels
//
#define LOGGER_CRIT(msg, ...) LOGGER_TRACE(LOG_CRIT, msg, ##__VA_ARGS__)
#define LOGGER_ERR(msg, ...) LOGGER_TRACE(LOG_ERR, msg, ##__VA_ARGS__)
#define LOGGER_INFO(msg, ...) LOGGER_TRACE(LOG_INFO, msg, ##__VA_ARGS__)
#define LOGGER_DEBUG(msg, ...) LOGGER_TRACE(LOG_DEBUG, msg, ##__VA_ARGS__)

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
