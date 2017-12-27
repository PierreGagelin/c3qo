

#include <syslog.h>


#define LOGGER_OPEN()  openlog("c3qo", 0, 0);
#define LOGGER_CLOSE() closelog();

#define LOGGER_DEBUG(msg, ...)  syslog(LOG_DEBUG,   msg, ##__VA_ARGS__);
#define LOGGER_INFO(msg, ...)   syslog(LOG_INFO,    msg, ##__VA_ARGS__);
#define LOGGER_NOTICE(msg, ...) syslog(LOG_NOTICE,  msg, ##__VA_ARGS__);
#define LOGGER_WARN(msg, ...)   syslog(LOG_WARNING, msg, ##__VA_ARGS__);
#define LOGGER_ERR(msg, ...)    syslog(LOG_ERR,     msg, ##__VA_ARGS__);


