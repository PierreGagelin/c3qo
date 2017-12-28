#ifndef C3QO_LOGGER_H_
#define C3QO_LOGGER_H_


#include <syslog.h> /* syslog */


/* Open and close connection to syslog */
#define LOGGER_OPEN()  openlog("c3qo", 0, 0);
#define LOGGER_CLOSE() closelog();

/* Format a log entry with function name, line and level */
#define LOGGER_DEBUG(msg, ...)  syslog(LOG_DEBUG,   "%s:%d [DEBUG] "   msg, __func__, __LINE__, ##__VA_ARGS__);
#define LOGGER_INFO(msg, ...)   syslog(LOG_INFO,    "%s:%d [INFO] "    msg, __func__, __LINE__, ##__VA_ARGS__);
#define LOGGER_NOTICE(msg, ...) syslog(LOG_NOTICE,  "%s:%d [NOTICE] "  msg, __func__, __LINE__, ##__VA_ARGS__);
#define LOGGER_WARN(msg, ...)   syslog(LOG_WARNING, "%s:%d [WARNING] " msg, __func__, __LINE__, ##__VA_ARGS__);
#define LOGGER_ERR(msg, ...)    syslog(LOG_ERR,     "%s:%d [ERROR] "   msg, __func__, __LINE__, ##__VA_ARGS__);


#endif


