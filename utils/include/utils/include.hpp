

//
// Common includes
//

// C++ headers
#include <cstring>
#include <unordered_map>
#include <vector>

// C headers
extern "C"
{
#include <getopt.h>
}

#define ASSERT(condition)         \
    do                            \
    {                             \
        if ((condition) == false) \
        {                         \
            exit(1);              \
        }                         \
    } while (false);
