

// Project headers
#include "c3qo/manager.hpp"

extern char *optarg; // Comes with getopt

int main(int argc, char **argv)
{
    struct manager mgr;
    int opt;
    char *filename;

    LOGGER_OPEN("c3qo");
    logger_set_level(LOGGER_LEVEL_DEBUG);

    filename = nullptr;
    while ((opt = getopt(argc, argv, "hf:l:")) != -1)
    {
        switch (opt)
        {
        case 'h':
        {
            LOGGER_DEBUG("CLI help: lol, help is for the weaks");
        }
        break;

        case 'f':
        {
            LOGGER_DEBUG("CLI file to load configuration from [path=%s]", optarg);
            filename = optarg;
        }
        break;

        case 'l':
        {
            enum logger_level level;

            errno = 0;
            level = (enum logger_level)strtol(optarg, nullptr, 10);
            if (errno != 0)
            {
                LOGGER_ERR("Failed to call strtol for log level: %s [errno=%d ; level=%s]", strerror(errno), errno, optarg);
                break;
            }

            LOGGER_DEBUG("CLI setting log level [level=%s]", get_logger_level(level));

            logger_set_level(level);
        }
        break;

        default:
            LOGGER_ERR("CLI argument error");
        }
    }

    // Parse configuration file
    if (filename != nullptr)
    {
        bool conf;

        conf = mgr.conf_parse(filename);
        if (conf == false)
        {
            // We don't care, it will be configured from socket
        }
    }

    // Main loop
    while (true)
    {
        mgr.fd_poll();
        mgr.timer_check_exp();
    }

    LOGGER_CLOSE();

    return 0;
}
