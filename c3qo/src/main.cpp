

// C++ library headers
#include <cstdlib>

// System library headers
extern "C" {
#include <unistd.h> // getopt
}

// Project headers
#include "c3qo/manager_bk.hpp"
#include "c3qo/manager_fd.hpp"
#include "c3qo/manager_tm.hpp"
#include "utils/logger.hpp"

// Managers shall be linked
extern class manager_bk m_bk;
extern class manager_tm m_tm;
extern class manager_fd m_fd;

extern char *optarg; // Comes with getopt

int main(int argc, char **argv)
{
    bool conf;
    int opt;
    const char *filename = "/tmp/config.txt";
    int ret = 0;

    LOGGER_OPEN("c3qo");
    logger_set_level(LOGGER_LEVEL_DEBUG);

    while ((opt = getopt(argc, argv, "hf:l:")) != -1)
    {
        switch (opt)
        {
        case 'h':
        {
            LOGGER_DEBUG("CLI help : lol, help is for the weaks");
            break;
        }
        case 'f':
        {
            LOGGER_DEBUG("CLI file to load configuration : %s", optarg);
            filename = optarg;
            break;
        }
        case 'l':
        {
            unsigned long int level;

            LOGGER_DEBUG("CLI setting log level to %s", optarg);

            level = strtoul(optarg, NULL, 10);
            logger_set_level((enum logger_level)level);

            break;
        }
        default:
        {
            LOGGER_ERR("CLI argument error");
            break;
        }
        }
    }

    // Parse configuration file
    conf = m_bk.conf_parse(filename);
    if (conf == false)
    {
        ret = -1;
    }

    // Main loop
    while (true)
    {
        m_fd.select_fd();
        m_tm.check_exp();
    }

    LOGGER_CLOSE();

    return ret;
}
