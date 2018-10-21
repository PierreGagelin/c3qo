

#define LOGGER_TAG "[app.network_cli]"

#include <wordexp.h>

// Project headers
#include "utils/logger.hpp"
#include "utils/socket.hpp"

// Generated protobuf command
#include "conf.pb-c.h"

extern int optind;
extern char *optarg;

// Convert a string to a command type
static PbcCmd__CmdType string_to_pbc_type(const char *string)
{
    if (strcmp(string, "add") == 0)
    {
        return PBC_CMD__CMD_TYPE__CMD_ADD;
    }
    else if (strcmp(string, "init") == 0)
    {
        return PBC_CMD__CMD_TYPE__CMD_INIT;
    }
    else if (strcmp(string, "conf") == 0)
    {
        return PBC_CMD__CMD_TYPE__CMD_CONF;
    }
    else if (strcmp(string, "bind") == 0)
    {
        return PBC_CMD__CMD_TYPE__CMD_BIND;
    }
    else if (strcmp(string, "start") == 0)
    {
        return PBC_CMD__CMD_TYPE__CMD_START;
    }
    else if (strcmp(string, "stop") == 0)
    {
        return PBC_CMD__CMD_TYPE__CMD_STOP;
    }
    else
    {
        return static_cast<PbcCmd__CmdType>(-1);
    }
}

//
// @brief Fills a ZMQ message to send a raw configuration line
//
static bool ncli_conf_proto(int argc, char **argv, struct c3qo_zmq_msg &msg)
{
    const char options[] = "a:i:t:";
    char *block_arg;
    int32_t block_id;
    PbcCmd__CmdType cmd_type;

    // Default parameters
    block_arg = nullptr;
    block_id = 1;
    cmd_type = PBC_CMD__CMD_TYPE__CMD_ADD;

    optind = 1; // reset getopt
    for (int opt = getopt(argc, argv, options); opt != -1; opt = getopt(argc, argv, options))
    {
        switch (opt)
        {
        case 'a':
            LOGGER_DEBUG("CLI: PROTO [block_arg=%s]", optarg);
            block_arg = optarg;
            break;

        case 'i':
            LOGGER_DEBUG("CLI: PROTO [block_id=%s]", optarg);
            block_id = static_cast<int32_t>(atoi(optarg));
            break;

        case 't':
            LOGGER_DEBUG("CLI: PROTO [cmd_type=%s]", optarg);
            cmd_type = string_to_pbc_type(optarg);
            break;

        default:
            LOGGER_WARNING("Unknown CLI option [opt=%c]", static_cast<char>(opt));
            return false;
        }
    }

    // Prepare a protobuf message
    PbcCmd cmd;
    pbc_cmd__init(&cmd);
    cmd.type = cmd_type;
    cmd.has_block_id = 1;
    cmd.block_id = block_id;
    cmd.block_arg = block_arg;

    size_t size = pbc_cmd__get_packed_size(&cmd);
    uint8_t *buffer = new uint8_t[size];
    pbc_cmd__pack(&cmd, buffer);

    // Fill ZMQ message
    msg.topic = strdup("CONF.PROTO.CMD");
    ASSERT(msg.topic != nullptr);
    msg.topic_len = strlen(msg.topic);

    msg.data = reinterpret_cast<char *>(buffer);
    msg.data_len = size;

    return true;
}

//
// @brief Fills a ZMQ message to send a raw configuration line
//
static bool ncli_conf_raw(int argc, char **argv, struct c3qo_zmq_msg &msg)
{
    const char options[] = "p:";
    const char *payload = "1 1 1";

    // Get CLI options
    optind = 1; // reset getopt
    for (int opt = getopt(argc, argv, options); opt != -1; opt = getopt(argc, argv, options))
    {
        switch (opt)
        {
        case 'p':
            LOGGER_DEBUG("CLI payload to send [payload=%s]", optarg);
            payload = optarg;
            break;

        default:
            LOGGER_ERR("Unknown CLI option [opt=%c]", static_cast<char>(opt));
            return false;
        }
    }

    // Fill ZMQ message
    msg.topic = strdup("CONF.LINE");
    ASSERT(msg.topic != nullptr);
    msg.topic_len = sizeof("CONF.LINE");

    msg.data = strdup(payload);
    ASSERT(msg.data != nullptr);
    msg.data_len = strlen(msg.data) + 1;

    return true;
}

int main(int argc, char **argv)
{
    const char options[] = "ha:T:A:";
    char *addr;
    char addr_def[] = "tcp://127.0.0.1:6666";
    char *ncli_type;
    char *ncli_args;
    int rc;

    LOGGER_OPEN("network_cli");
    logger_set_level(LOGGER_LEVEL_DEBUG);

    // Get CLI options
    addr = nullptr;
    ncli_type = nullptr;
    ncli_args = nullptr;
    for (int opt = getopt(argc, argv, options); opt != -1; opt = getopt(argc, argv, options))
    {
        switch (opt)
        {
        case 'h':
            LOGGER_DEBUG("CLI help: lol, help is for the weaks");
            return 0;

        case 'a':
            LOGGER_DEBUG("CLI connection address for the : %s", optarg);
            addr = optarg;
            break;

        case 'A':
            LOGGER_DEBUG("CLI arguments for the network CLI : %s", optarg);
            ncli_args = optarg;
            break;

        case 'T':
            LOGGER_DEBUG("CLI type of network CLI to execute : %s", optarg);
            ncli_type = optarg;
            break;

        default:
            LOGGER_ERR("Unknown CLI option [opt=%c]", static_cast<char>(opt));
            return 1;
        }
    }

    // Verify input
    if (addr == nullptr)
    {
        LOGGER_DEBUG("No connection address given by CLI, using default [addr=%s]", addr_def);
        addr = addr_def;
    }
    ASSERT(ncli_type != nullptr);
    ASSERT(ncli_args != nullptr);

    // Shell expansion of 'A' option in an array of arguments
    wordexp_t we;
    ASSERT(wordexp(ncli_args, &we, 0) == 0);

    struct c3qo_zmq_msg msg;
    if (strcmp(ncli_type, "raw") == 0)
    {
        ASSERT(ncli_conf_raw(we.we_wordc, we.we_wordv, msg) == true);
    }
    else if (strcmp(ncli_type, "proto") == 0)
    {
        ASSERT(ncli_conf_proto(we.we_wordc, we.we_wordv, msg) == true);
    }
    else
    {
        ASSERT(true == false);
    }

    wordfree(&we);

    void *ctx = zmq_ctx_new();
    ASSERT(ctx != nullptr);

    // Create a client
    void *client = zmq_socket(ctx, ZMQ_PAIR);
    ASSERT(client != nullptr);

    // Create a socket to monitor another one
    void *monitor = zmq_socket(ctx, ZMQ_PAIR);
    ASSERT(monitor != nullptr);

    // Monitor the socket
    {
        // Filter to receive only accepted connection event
        rc = zmq_socket_monitor(client, "inproc://monitor-pair", ZMQ_EVENT_CONNECTED);
        ASSERT(rc != -1);

        rc = zmq_connect(monitor, "inproc://monitor-pair");
        ASSERT(rc != -1);
    }

    // Connect the socket
    rc = zmq_connect(client, addr);
    ASSERT(rc != -1);

    // Wait for the client to be connected
    rc = socket_zmq_get_event(monitor);
    ASSERT(rc != -1);

    // Send a two-parts message
    socket_zmq_write(client, msg.topic, msg.topic_len, ZMQ_SNDMORE);
    socket_zmq_write(client, msg.data, msg.data_len, 0);

    free(msg.topic);
    free(msg.data);

    // Close down the sockets
    zmq_close(client);
    zmq_close(monitor);
    zmq_ctx_term(ctx);

    LOGGER_CLOSE();

    return 0;
}
