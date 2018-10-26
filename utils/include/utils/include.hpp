// Useful includes

// C headers
extern "C"
{
#include <dlfcn.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <syslog.h>
#include <unistd.h>
#include <zmq.h>
}

// C++ headers
#include <cerrno>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <forward_list>
#include <fstream>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#define ASSERT(condition)         \
    do                            \
    {                             \
        if ((condition) == false) \
        {                         \
            exit(1);              \
        }                         \
    } while (false);
