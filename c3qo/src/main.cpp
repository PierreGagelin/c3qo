

#define LOGGER_TAG "[app.network_cli]"

// Project headers
#include "c3qo/manager.hpp"

extern char *optarg; // Comes with getopt

bool end_signal_received = false;

static void signal_handler(int sig, siginfo_t *, void *)
{
    if ((sig == SIGTERM) || (sig == SIGINT))
    {
        LOGGER_NOTICE("Received signal to end program [signum=%d]");
        end_signal_received = true;
    }
    else
    {
        LOGGER_ERR("Received unexpected signal [signum=%d]", sig);
    }
}

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

    // Register a signal handler
    {
        struct sigaction sa;

        sa.sa_flags = SA_SIGINFO;
        sigemptyset(&sa.sa_mask);
        sa.sa_sigaction = signal_handler;
        if (sigaction(SIGINT, &sa, NULL) == -1)
        {
            LOGGER_ERR("Failed to register signal handler: %s [signal=SIGINT ; errno=%d]", strerror(errno), errno);
            return 1;
        }
        LOGGER_NOTICE("Registered signal handler for SIGINT");
        if (sigaction(SIGTERM, &sa, NULL) == -1)
        {
            LOGGER_ERR("Failed to register signal handler: %s [signal=SIGTERM ; errno=%d]", strerror(errno), errno);
            return 1;
        }
        LOGGER_NOTICE("Registered signal handler for SIGTERM");
    }

    // Main loop
    while (end_signal_received == false)
    {
        mgr.fd_poll();
        mgr.timer_check_exp();
    }

    LOGGER_CLOSE();

    return 0;
}
