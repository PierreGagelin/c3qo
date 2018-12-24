

// Project headers
#include "block/hello.hpp"
#include "block/trans_pb.hpp"
#include "block/zmq_pair.hpp"
#include "engine/manager.hpp"

// C headers
extern "C"
{
#include <signal.h>
}

extern char *optarg; // Comes with getopt

bool end_signal_received = false;

static void signal_handler(int sig, siginfo_t *, void *)
{
    if ((sig == SIGTERM) || (sig == SIGINT))
    {
        LOGGER_NOTICE("Received signal to end program [signum=%d]", sig);
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

    LOGGER_OPEN("c3qo");
    logger_set_level(LOGGER_LEVEL_DEBUG);

    while ((opt = getopt(argc, argv, "hl:")) != -1)
    {
        switch (opt)
        {
        case 'h':
            LOGGER_DEBUG("CLI help: lol, help is for the weaks");
            return 0;

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
            return 1;
        }
    }

    // Register block factories
    struct hello_factory hello;
    struct trans_pb_factory trans_pb;
    struct zmq_pair_factory zmq_pair;
    mgr.block_factory_register("hello", &hello);
    mgr.block_factory_register("trans_pb", &trans_pb);
    mgr.block_factory_register("zmq_pair", &zmq_pair);

    // Add the ZMQ monitoring server
    struct zmq_pair *block;
    mgr.block_add(-1, "zmq_pair");
    block = static_cast<struct zmq_pair *>(mgr.block_get(-1));
    block->addr_ = "tcp://127.0.0.1:1664";
    block->client_ = false;
    mgr.block_start(-1);

    // Add the protobuf transcoder
    mgr.block_add(-2, "trans_pb");
    mgr.block_start(-2);

    // Bind ZMQ and transcoder
    mgr.block_bind(-1, 1, -2);
    mgr.block_bind(-2, 1, -1);

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
