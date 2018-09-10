

#define LOGGER_TAG "[lib.logger]"

// Project headers
#include "utils/logger.hpp"

// Current level of log
enum logger_level logger_level;

//
// @brief Return a string from logger level
//
const char *get_logger_level(enum logger_level l)
{
    switch (l)
    {
    case LOGGER_LEVEL_NONE:
        return "LOG_NONE";

    case LOGGER_LEVEL_EMERG:
        return "LOG_EMERG";

    case LOGGER_LEVEL_ALERT:
        return "LOG_ALERT";

    case LOGGER_LEVEL_CRIT:
        return "LOG_CRIT";

    case LOGGER_LEVEL_ERR:
        return "LOG_ERR";

    case LOGGER_LEVEL_WARNING:
        return "LOG_WARNING";

    case LOGGER_LEVEL_NOTICE:
        return "LOG_NOTICE";

    case LOGGER_LEVEL_INFO:
        return "LOG_INFO";

    case LOGGER_LEVEL_DEBUG:
        return "LOG_DEBUG";

    default:
        return "LOG_UNKNOWN";
    }
}

//
// @brief Set the level of log
//
void logger_set_level(enum logger_level l)
{
    // Verify input
    if (l < 0)
    {
        logger_level = LOGGER_LEVEL_NONE;
        LOGGER_TRACE(LOG_WARNING, "Cannot set logger level lower than minimum. Level set to minimum [asked=%d ; set=%s]", l, get_logger_level(logger_level));
        return;
    }
    if (l > LOGGER_LEVEL_DEBUG)
    {
        logger_level = LOGGER_LEVEL_DEBUG;
        LOGGER_TRACE(LOG_WARNING, "Cannot set logger level higher than maximum. Level set to maximum [asked=%d ; set=%s]", l, get_logger_level(logger_level));
        return;
    }

    LOGGER_TRACE(LOG_INFO, "Setting log level [old=%s ; new=%s]", get_logger_level(logger_level), get_logger_level(l));

    logger_level = l;
}
