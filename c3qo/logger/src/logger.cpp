

// Project headers
#include "c3qo/logger.hpp"

enum logger_level logger_level;

void logger_set_level(enum logger_level l)
{
    syslog(LOG_INFO, "Setting log level from %d to %d", logger_level, l);

    if (l > LOGGER_LEVEL_MAX)
    {
        logger_level = LOGGER_LEVEL_MAX;
        LOGGER_WARNING("Asked to set logger level higher than maximum. Level set to maximum");
        return;
    }

    logger_level = l;
}
