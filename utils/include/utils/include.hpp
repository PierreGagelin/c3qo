

//
// Common includes
//

// Project headers
#include "utils/logger.hpp"

// C++ headers
#include <cstring>
#include <unordered_map>
#include <vector>

// C headers
extern "C"
{
#include <getopt.h>
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
