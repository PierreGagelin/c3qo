

// C++ library headers
#include <cstdlib> // NULL
#include <cstring> // memcpy

// System library headers
extern "C" {
#include <unistd.h> // getopt
}

// Project headers
#include "utils/logger.hpp"
#include "utils/socket.hpp"

int main(int argc, char **argv)
{
    int opt;
    char *addr;
    char *topic;
    char *payload;
    char addr_def[] = "tcp://127.0.0.1:6666";
    char topic_def[] = "CONF.LINE";
    char payload_def[] = "1 1 1";
    int rc;

    LOGGER_OPEN("network_cli");
    logger_set_level(LOGGER_LEVEL_DEBUG);

    // Get specifications from CLI
    addr = NULL;
    topic = NULL;
    payload = NULL;
    while ((opt = getopt(argc, argv, "ha:l:p:t:")) != -1)
    {
        switch (opt)
        {
        case 'h':
        {
            LOGGER_DEBUG("CLI help: lol, help is for the weaks");
        }
        break;

        case 'a':
        {
            LOGGER_DEBUG("CLI connection address for the : %s", optarg);
            addr = optarg;
        }
        break;

        case 'l':
        {
            enum logger_level level;

            errno = 0;
            level = (enum logger_level)strtol(optarg, NULL, 10);
            if (errno != 0)
            {
                LOGGER_ERR("Failed to set log from CLI [level=%s]", optarg);
                break;
            }

            logger_set_level(level);
        }
        break;

        case 'p':
        {
            LOGGER_DEBUG("CLI payload to send [payload=%s]", optarg);
            payload = optarg;
        }
        break;

        case 't':
        {
            LOGGER_DEBUG("CLI topic to send [topic=%s]", optarg);
            topic = optarg;
        }
        break;

        default:
            LOGGER_WARNING("Unknown CLI option [opt=%c]", (char)opt);
        }
    }

    // Verify input
    {
        if (addr == NULL)
        {
            LOGGER_DEBUG("No connection address given by CLI, using default [addr=%s]", addr_def);
            addr = addr_def;
        }
        if (payload == NULL)
        {
            LOGGER_DEBUG("No configuration line given by CLI, using default [payload=%s]", payload_def);
            payload = payload_def;
        }
        if (topic == NULL)
        {
            LOGGER_DEBUG("No configuration topic given by CLI, using default [topic=%s]", topic_def);
            topic = topic_def;
        }
    }

    void *ctx = zmq_ctx_new();
    if (ctx == NULL)
    {
        LOGGER_ERR("Failed");
        return 1;
    }

    // Create a publisher
    void *publisher = zmq_socket(ctx, ZMQ_PAIR);
    if (publisher == NULL)
    {
        LOGGER_ERR("Failed");
        return 1;
    }

    // Create a socket to monitor another one
    void *monitor = zmq_socket(ctx, ZMQ_PAIR);
    if (monitor == NULL)
    {
        LOGGER_ERR("Failed");
        return 1;
    }

    // Monitor the socket
    {
        // Filter to receive only accepted connection event
        rc = zmq_socket_monitor(publisher, "inproc://monitor-pair", ZMQ_EVENT_ACCEPTED);
        if (rc == -1)
        {
            LOGGER_ERR("Failed");
            return 1;
        }

        rc = zmq_connect(monitor, "inproc://monitor-pair");
        if (rc == -1)
        {
            LOGGER_ERR("Failed");
            return 1;
        }
    }

    // Bind the socket
    rc = zmq_bind(publisher, addr);
    if (rc == -1)
    {
        LOGGER_ERR("Failed");
        return 1;
    }

    // Wait for an accepted client
    rc = socket_zmq_get_event(monitor);
    if (rc == -1)
    {
        LOGGER_ERR("Failed");
        return 1;
    }

    // Send a two-parts message
    socket_zmq_write(publisher, topic, strlen(topic), ZMQ_SNDMORE);
    socket_zmq_write(publisher, payload, strlen(payload), 0);

    // Close down the sockets
    zmq_close(publisher);
    zmq_close(monitor);
    zmq_ctx_term(ctx);

    LOGGER_CLOSE();

    return 0;
}
