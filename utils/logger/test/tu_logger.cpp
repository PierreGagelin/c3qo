//
// @brief Test file for the block manager
//

#define LOGGER_TAG "[TU.logger]"

// Project headers
#include "utils/logger.hpp"

// Logger library should be linked
extern enum logger_level logger_level;

//
// @brief Test level of log
//
static void tu_logger_level()
{
    // Regular level of logs
    logger_set_level(LOGGER_LEVEL_DEBUG);
    ASSERT(logger_level == LOGGER_LEVEL_DEBUG);
    get_logger_level(LOGGER_LEVEL_DEBUG);

    logger_set_level(LOGGER_LEVEL_INFO);
    ASSERT(logger_level == LOGGER_LEVEL_INFO);
    get_logger_level(LOGGER_LEVEL_INFO);

    logger_set_level(LOGGER_LEVEL_NOTICE);
    ASSERT(logger_level == LOGGER_LEVEL_NOTICE);
    get_logger_level(LOGGER_LEVEL_NOTICE);

    logger_set_level(LOGGER_LEVEL_WARNING);
    ASSERT(logger_level == LOGGER_LEVEL_WARNING);
    get_logger_level(LOGGER_LEVEL_WARNING);

    logger_set_level(LOGGER_LEVEL_ERR);
    ASSERT(logger_level == LOGGER_LEVEL_ERR);
    get_logger_level(LOGGER_LEVEL_ERR);

    logger_set_level(LOGGER_LEVEL_CRIT);
    ASSERT(logger_level == LOGGER_LEVEL_CRIT);
    get_logger_level(LOGGER_LEVEL_CRIT);

    logger_set_level(LOGGER_LEVEL_ALERT);
    ASSERT(logger_level == LOGGER_LEVEL_ALERT);
    get_logger_level(LOGGER_LEVEL_ALERT);

    logger_set_level(LOGGER_LEVEL_EMERG);
    ASSERT(logger_level == LOGGER_LEVEL_EMERG);
    get_logger_level(LOGGER_LEVEL_EMERG);

    logger_set_level(LOGGER_LEVEL_NONE);
    ASSERT(logger_level == LOGGER_LEVEL_NONE);
    get_logger_level(LOGGER_LEVEL_NONE);

    // Corrupted levels
    logger_set_level(static_cast<enum logger_level>(666));
    ASSERT(logger_level == LOGGER_LEVEL_DEBUG);
    get_logger_level(static_cast<enum logger_level>(666));

    logger_set_level(static_cast<enum logger_level>(-666));
    ASSERT(logger_level == LOGGER_LEVEL_NONE);
    get_logger_level(static_cast<enum logger_level>(-666));
}

int main(int, char **)
{
    LOGGER_OPEN("tu_logger");
    logger_set_level(LOGGER_LEVEL_DEBUG);

    tu_logger_level();

    LOGGER_CLOSE();
    return 0;
}
