

// C++ library headers
#include <cstdlib>

// System library headers
extern "C" {
#include <unistd.h> // getopt
}

// Project headers
#include "c3qo/manager.hpp"
#include "utils/logger.hpp"

extern char *optarg; // Comes with getopt

int main(int argc, char **argv)
{
    bool conf;
    int opt;
    char *filename;

    LOGGER_OPEN("c3qo");
    logger_set_level(LOGGER_LEVEL_DEBUG);

    filename = NULL;
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

    // Export the manager
    m = new struct manager;

    // Parse configuration file
    if (filename != NULL)
    {
        conf = m->bk.conf_parse(filename);
        if (conf == false)
        {
            // We don't care, it will be configured from socket
        }
    }

    // Main loop
    while (true)
    {
        m->fd.poll_fd();
        m->tm.check_exp();
    }

    LOGGER_CLOSE();

    delete m;

    return 0;
}
