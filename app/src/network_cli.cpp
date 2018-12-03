

#define LOGGER_TAG "[app.network_cli]"

#include <wordexp.h>

// Project headers
#include "utils/logger.hpp"
#include "utils/socket.hpp"

// Generated protobuf command
#include "conf.pb-c.h"

extern int optind;
extern char *optarg;

//
// @brief Fills a ZMQ message to send a raw configuration line
//
static bool ncli_conf_proto(int argc, char **argv, std::vector<struct c3qo_zmq_part> &msg)
{
    const char options[] = "a:d:i:p:t:";
    char *block_arg;
    int32_t block_id;
    int32_t port;
    int32_t dest;
    const char *type;

    // Default parameters
    block_arg = nullptr;
    block_id = 1;
    port = 1;
    dest = 1;
    type = "unknown";

    optind = 1; // reset getopt
    for (int opt = getopt(argc, argv, options); opt != -1; opt = getopt(argc, argv, options))
    {
        switch (opt)
        {
        case 'a':
            LOGGER_DEBUG("CLI: PROTO [block_arg=%s]", optarg);
            block_arg = optarg;
            break;

        case 'd':
            LOGGER_DEBUG("CLI: PROTO [dest=%s]", optarg);
            dest = static_cast<int32_t>(atoi(optarg));
            break;

        case 'i':
            LOGGER_DEBUG("CLI: PROTO [block_id=%s]", optarg);
            block_id = static_cast<int32_t>(atoi(optarg));
            break;

        case 'p':
            LOGGER_DEBUG("CLI: PROTO [port=%s]", optarg);
            port = static_cast<int32_t>(atoi(optarg));
            break;

        case 't':
            LOGGER_DEBUG("CLI: PROTO [cmd_type=%s]", optarg);
            type = optarg;
            break;

        default:
            LOGGER_WARNING("Unknown CLI option [opt=%c]", static_cast<char>(opt));
            return false;
        }
    }

    // Prepare a protobuf message
    Command cmd;
    BlockAdd add;
    BlockStart start;
    BlockStop stop;
    BlockDel del;
    BlockConf conf;
    BlockBind bind;
    command__init(&cmd);

    if (strcmp(type, "add") == 0)
    {
        cmd.type_case = COMMAND__TYPE_ADD;
        block_add__init(&add);
        cmd.add = &add;
        cmd.add->id = block_id;
        cmd.add->type = block_arg;
    }
    else if (strcmp(type, "start") == 0)
    {
        cmd.type_case = COMMAND__TYPE_START;
        block_start__init(&start);
        cmd.start = &start;
        cmd.start->id = block_id;
    }
    else if (strcmp(type, "stop") == 0)
    {
        cmd.type_case = COMMAND__TYPE_STOP;
        block_stop__init(&stop);
        cmd.stop = &stop;
        cmd.stop->id = block_id;
    }
    else if (strcmp(type, "del") == 0)
    {
        cmd.type_case = COMMAND__TYPE_DEL;
        block_del__init(&del);
        cmd.del = &del;
        cmd.del->id = block_id;
    }
    else if (strcmp(type, "conf") == 0)
    {
        cmd.type_case = COMMAND__TYPE_CONF;
        block_conf__init(&conf);
        cmd.conf = &conf;
        cmd.conf->id = block_id;
        cmd.conf->conf = block_arg;
    }
    else if (strcmp(type, "bind") == 0)
    {
        cmd.type_case = COMMAND__TYPE_BIND;
        block_bind__init(&bind);
        cmd.bind = &bind;
        cmd.bind->id = block_id;
        cmd.bind->port = port;
        cmd.bind->dest = dest;
    }
    else
    {
        cmd.type_case = COMMAND__TYPE__NOT_SET;
    }

    size_t size = command__get_packed_size(&cmd);
    uint8_t *buffer = new uint8_t[size];
    command__pack(&cmd, buffer);

    // Fill ZMQ message
    struct c3qo_zmq_part part;
    part.data = strdup("CONF.PROTO.CMD");
    ASSERT(part.data != nullptr);
    part.len = strlen(part.data);
    msg.push_back(part);

    part.data = reinterpret_cast<char *>(buffer);
    part.len = size;
    msg.push_back(part);

    return true;
}

int main(int argc, char **argv)
{
    const char options[] = "A:";
    char *ncli_args;
    int rc;
    bool ok;

    LOGGER_OPEN("network_cli");
    logger_set_level(LOGGER_LEVEL_DEBUG);

    // Get CLI options
    ncli_args = nullptr;
    for (int opt = getopt(argc, argv, options); opt != -1; opt = getopt(argc, argv, options))
    {
        switch (opt)
        {
        case 'A':
            LOGGER_DEBUG("CLI arguments for the network CLI : %s", optarg);
            ncli_args = optarg;
            break;

        default:
            LOGGER_ERR("Unknown CLI option [opt=%c]", static_cast<char>(opt));
            return 1;
        }
    }

    // Verify input
    ASSERT(ncli_args != nullptr);

    // Shell expansion of 'A' option in an array of arguments
    wordexp_t we;
    ASSERT(wordexp(ncli_args, &we, 0) == 0);

    std::vector<struct c3qo_zmq_part> msg;
    ok = ncli_conf_proto(we.we_wordc, we.we_wordv, msg);
    ASSERT(ok == true);

    wordfree(&we);

    void *ctx = zmq_ctx_new();
    ASSERT(ctx != nullptr);

    // Create a client
    void *client = zmq_socket(ctx, ZMQ_PAIR);
    ASSERT(client != nullptr);

    // Create a socket to monitor another one
    void *monitor = zmq_socket(ctx, ZMQ_PAIR);
    ASSERT(monitor != nullptr);

    // Filter to receive only accepted connection event
    rc = zmq_socket_monitor(client, "inproc://monitor-pair", ZMQ_EVENT_CONNECTED);
    ASSERT(rc != -1);

    rc = zmq_connect(monitor, "inproc://monitor-pair");
    ASSERT(rc != -1);

    // Connect the socket
    rc = zmq_connect(client, "tcp://127.0.0.1:1664");
    ASSERT(rc != -1);

    // Wait for the client to be connected
    rc = socket_zmq_get_event(monitor);
    ASSERT(rc != -1);

    // Send a two-parts message
    ok = socket_zmq_write(client, msg);
    ASSERT(ok == true);

    c3qo_zmq_msg_del(msg);

    // Close down the sockets
    zmq_close(client);
    zmq_close(monitor);
    zmq_ctx_term(ctx);

    LOGGER_CLOSE();

    return 0;
}
