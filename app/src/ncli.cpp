

extern "C"
{
#include <wordexp.h>
}

// Project headers
#include "engine/manager.hpp"
#include "utils/logger.hpp"
#include "utils/socket.hpp"

// Generated protobuf command
#include "conf.pb-c.h"

// Local headers
#include "ncli.hpp"

extern int optind;
extern char *optarg;

ncli::ncli(struct manager *mgr) : block(mgr) {}
ncli::~ncli() {}

//
// @brief Fills a ZMQ message to send a raw configuration line
//
static bool ncli_conf_proto(int argc, char **argv, std::vector<struct c3qo_zmq_part> &msg)
{
    const char *options;
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
    options = "a:d:i:p:t:";
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

void ncli::start_()
{
    bool ok;
    int rc;

    //
    // ZeroMQ setup
    //
    zmq_ctx_ = zmq_ctx_new();
    ASSERT(zmq_ctx_ != nullptr);

    zmq_sock_.socket = zmq_socket(zmq_ctx_, ZMQ_PAIR);
    ASSERT(zmq_sock_.socket != nullptr);

    rc = zmq_connect(zmq_sock_.socket, "tcp://127.0.0.1:1665");
    ASSERT(rc != -1);

    zmq_sock_.bk = this;
    zmq_sock_.fd = -1;
    zmq_sock_.read = true;
    zmq_sock_.write = false;
    mgr_->fd_add(zmq_sock_);

    //
    // Prepare and send the network command
    //
    std::vector<struct c3qo_zmq_part> msg;
    struct c3qo_zmq_part dest;
    wordexp_t we;

    ASSERT(wordexp(ncli_args_, &we, 0) == 0);

    dest.data = strdup(ncli_peer_);
    dest.len = strlen(dest.data) + 1;
    msg.push_back(dest);

    ok = ncli_conf_proto(we.we_wordc, we.we_wordv, msg);
    ASSERT(ok == true);

    wordfree(&we);

    ok = socket_zmq_write(zmq_sock_.socket, msg);
    ASSERT(ok == true);

    c3qo_zmq_msg_del(msg);

    //
    // Set a timeout upon answer reception
    //
    struct timer tm;
    tm.arg = nullptr;
    tm.bk = this;
    tm.tid = 1;
    tm.time.tv_nsec = 0;
    tm.time.tv_sec = 1;
    mgr_->timer_add(tm);
}

void ncli::stop_()
{
    // Unregister the socket
    mgr_->fd_remove(zmq_sock_);

    // ZeroMQ teardown
    zmq_close(zmq_sock_.socket);
    zmq_ctx_term(zmq_ctx_);
}

void ncli::on_fd_(struct file_desc &)
{
    std::vector<struct c3qo_zmq_part> msg;

    socket_zmq_read(zmq_sock_.socket, msg);
    if (msg.size() != 3u)
    {
        LOGGER_DEBUG("Discard message: unexpected parts count [expected=3 ; actual=%zu]", msg.size());
        return;
    }
    if (msg[0].data != std::string(ncli_peer_))
    {
        LOGGER_DEBUG("Discard message: unexpected peer [expected=%s ; actual=%s]", ncli_peer_, msg[0].data);
        return;
    }
    if (msg[1].data != std::string("CONF.PROTO.CMD.REP"))
    {
        LOGGER_DEBUG("Discard message: unexpected topic [expected=CONF.PROTO.CMD.REP ; actual=%s]", msg[1].data);
        return;
    }
    if (msg[2].data != std::string("OK"))
    {
        LOGGER_DEBUG("Discard message: unexpected status [expected=OK ; actual=%s]", msg[2].data);
        return;
    }

    LOGGER_DEBUG("Received expected answer");
    received_answer_ = true;
}

void ncli::on_timer_(struct timer &)
{
    LOGGER_ERR("Failed to receive message before timeout expiration");
    timeout_expired_ = true;
}

int main(int argc, char **argv)
{
    struct manager mgr;
    struct ncli network_cli(&mgr);
    const char *options;

    LOGGER_OPEN("ncli");
    logger_set_level(LOGGER_LEVEL_DEBUG);

    // Get CLI options
    options = "A:i:";
    network_cli.ncli_args_ = nullptr;
    network_cli.ncli_peer_ = nullptr;
    for (int opt = getopt(argc, argv, options); opt != -1; opt = getopt(argc, argv, options))
    {
        switch (opt)
        {
        case 'A':
            LOGGER_DEBUG("Set network option request [value=%s]", optarg);
            network_cli.ncli_args_ = optarg;
            break;

        case 'i':
            LOGGER_DEBUG("Set identity of the peer [value=%s]", optarg);
            network_cli.ncli_peer_ = optarg;
            break;

        default:
            LOGGER_CRIT("Failed to parse option: unknown option [opt=%c]", static_cast<char>(opt));
            return 1;
        }
    }

    // Verify input
    ASSERT(network_cli.ncli_args_ != nullptr);
    ASSERT(network_cli.ncli_peer_ != nullptr);

    network_cli.start_();

    network_cli.received_answer_ = false;
    network_cli.timeout_expired_ = false;
    while ((network_cli.received_answer_ == false) && (network_cli.timeout_expired_ == false))
    {
        mgr.fd_poll();
        mgr.timer_check_exp();
    }

    network_cli.stop_();

    LOGGER_CLOSE();

    return network_cli.received_answer_ ? 0 : 1;
}
