#ifndef C3QO_LOGGER_H_
#define C3QO_LOGGER_H_


#include <syslog.h> /* syslog */


/* Open and close connection to syslog */
#define LOGGER_OPEN()  openlog("c3qo", 0, 0);
#define LOGGER_CLOSE() closelog();

/**
 * MAN SYSLOG :
 *   - LOG_EMERG   : system is unusable
 *   - LOG_ALERT   : action must be taken immediately
 *   - LOG_CRIT    : critical conditions
 *   - LOG_ERR     : error conditions
 *   - LOG_WARNING : warning conditions
 *   - LOG_NOTICE  : normal, but significant, condition
 *   - LOG_INFO    : informational message
 *   - LOG_DEBUG   : debug-level message
 */

/* Format a log entry with function name, line and level */
#define LOGGER_EMERG(msg, ...)   syslog(LOG_EMERG,   "%s:%d [EMERG] "   msg, __func__, __LINE__, ##__VA_ARGS__);
#define LOGGER_ALERT(msg, ...)   syslog(LOG_ALERT,   "%s:%d [ALERT] "   msg, __func__, __LINE__, ##__VA_ARGS__);
#define LOGGER_CRIT(msg, ...)    syslog(LOG_CRIT,    "%s:%d [CRIT] "    msg, __func__, __LINE__, ##__VA_ARGS__);
#define LOGGER_ERR(msg, ...)     syslog(LOG_ERR,     "%s:%d [ERROR] "   msg, __func__, __LINE__, ##__VA_ARGS__);
#define LOGGER_WARNING(msg, ...) syslog(LOG_WARNING, "%s:%d [WARNING] " msg, __func__, __LINE__, ##__VA_ARGS__);
#define LOGGER_NOTICE(msg, ...)  syslog(LOG_NOTICE,  "%s:%d [NOTICE] "  msg, __func__, __LINE__, ##__VA_ARGS__);
#define LOGGER_INFO(msg, ...)    syslog(LOG_INFO,    "%s:%d [INFO] "    msg, __func__, __LINE__, ##__VA_ARGS__);
#define LOGGER_DEBUG(msg, ...)   syslog(LOG_DEBUG,   "%s:%d [DEBUG] "   msg, __func__, __LINE__, ##__VA_ARGS__);


#endif


