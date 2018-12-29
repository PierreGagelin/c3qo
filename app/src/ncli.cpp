

extern "C"
{
#include <wordexp.h>
}

// Project headers
#include "block/hook_zmq.hpp"
#include "engine/manager.hpp"
#include "utils/logger.hpp"
#include "utils/buffer.hpp"

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
static bool ncli_conf_proto(int argc, char **argv, struct buffer &buf)
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
            LOGGER_DEBUG("Set command option [block_arg=%s]", optarg);
            block_arg = optarg;
            break;

        case 'd':
            LOGGER_DEBUG("Set command option [dest=%s]", optarg);
            dest = static_cast<int32_t>(atoi(optarg));
            break;

        case 'i':
            LOGGER_DEBUG("Set command option [block_id=%s]", optarg);
            block_id = static_cast<int32_t>(atoi(optarg));
            break;

        case 'p':
            LOGGER_DEBUG("Set command option [port=%s]", optarg);
            port = static_cast<int32_t>(atoi(optarg));
            break;

        case 't':
            LOGGER_DEBUG("Set command option [cmd_type=%s]", optarg);
            type = optarg;
            break;

        default:
            LOGGER_ERR("Failed to set command option: unknown option [opt=%c]", static_cast<char>(opt));
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
    else if (strcmp(type, "term") == 0)
    {
        cmd.type_case = COMMAND__TYPE_TERM;
        cmd.term = true;
    }
    else
    {
        cmd.type_case = COMMAND__TYPE__NOT_SET;
    }

    size_t size = command__get_packed_size(&cmd);
    uint8_t *buffer = new uint8_t[size];
    command__pack(&cmd, buffer);

    // Fill ZMQ message
    const char *topic = "CONF.PROTO.CMD";
    buf.push_back(topic, strlen(topic));

    buf.push_back(buffer, size);

    return true;
}

void ncli::start_()
{
    bool ok;

    //
    // Prepare and send the network command
    //
    struct buffer buf;
    wordexp_t we;

    ASSERT(wordexp(ncli_args_, &we, 0) == 0);

    buf.push_back(ncli_peer_, strlen(ncli_peer_) + 1);

    ok = ncli_conf_proto(we.we_wordc, we.we_wordv, buf);
    ASSERT(ok == true);

    wordfree(&we);

    process_data_(1, &buf);

    buf.clear();

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
    // Remove timer
    struct timer tm;
    tm.bk = this;
    tm.tid = 1;
    mgr_->timer_del(tm);
}

int ncli::data_(void *vdata)
{
    struct buffer &buf = *(static_cast<struct buffer *>(vdata));

    if (buf.parts_.size() != 3u)
    {
        LOGGER_DEBUG("Discard message: wrong parts count [expected=3 ; actual=%zu]", buf.parts_.size());
        return PORT_STOP;
    }
    if ((memcmp(buf.parts_[0].data, ncli_peer_, buf.parts_[0].len) != 0) ||
        (memcmp(buf.parts_[1].data, "CONF.PROTO.CMD.REP", buf.parts_[1].len) != 0) ||
        (memcmp(buf.parts_[2].data, "OK", buf.parts_[2].len) != 0))
    {
        LOGGER_DEBUG("Discard unexpected message [peer=%s ; topic=%s ; status=%s]",
                     static_cast<char *>(buf.parts_[0].data),
                     static_cast<char *>(buf.parts_[1].data),
                     static_cast<char *>(buf.parts_[2].data));
        return PORT_STOP;
    }

    LOGGER_INFO("Received expected answer");
    received_answer_ = true;

    return PORT_STOP;
}

void ncli::on_timer_(struct timer &)
{
    LOGGER_ERR("Failed to receive message before timeout expiration");
    timeout_expired_ = true;
}

struct block *ncli_factory::constructor(struct manager *mgr)
{
    return new struct ncli(mgr);
}

void ncli_factory::destructor(struct block *bk)
{
    delete static_cast<struct ncli *>(bk);
}

int main(int argc, char **argv)
{
    struct manager mgr;

    LOGGER_OPEN("ncli");

    //
    // Create
    //
    struct hook_zmq_factory hook_zmq_f;
    struct hook_zmq *hook;
    mgr.block_factory_register("hook_zmq", &hook_zmq_f);
    mgr.block_add(1, "hook_zmq");
    hook = static_cast<struct hook_zmq *>(mgr.block_get(1));
    ASSERT(hook != nullptr);

    struct ncli_factory ncli_f;
    struct ncli *cli;
    mgr.block_factory_register("ncli", &ncli_f);
    mgr.block_add(2, "ncli");
    cli = static_cast<struct ncli *>(mgr.block_get(2));
    ASSERT(cli != nullptr);

    //
    // Configure
    //
    hook->client_ = true;
    hook->type_ = ZMQ_PAIR;
    hook->addr_ = std::string("tcp://127.0.0.1:1665");

    const char *options;
    options = "A:i:";
    cli->ncli_args_ = nullptr;
    cli->ncli_peer_ = nullptr;
    for (int opt = getopt(argc, argv, options); opt != -1; opt = getopt(argc, argv, options))
    {
        switch (opt)
        {
        case 'A':
            LOGGER_DEBUG("Set network option request [value=%s]", optarg);
            cli->ncli_args_ = optarg;
            break;

        case 'i':
            LOGGER_DEBUG("Set identity of the peer [value=%s]", optarg);
            cli->ncli_peer_ = optarg;
            break;

        default:
            LOGGER_CRIT("Failed to parse option: unknown option [opt=%c]", static_cast<char>(opt));
            return 1;
        }
    }
    ASSERT(cli->ncli_args_ != nullptr);
    ASSERT(cli->ncli_peer_ != nullptr);

    //
    // Bind and start
    //
    mgr.block_bind(1, 1, 2);
    mgr.block_bind(2, 1, 1);

    mgr.block_start(1);
    mgr.block_start(2);

    //
    // Wait for answer or timeout
    //
    cli->received_answer_ = false;
    cli->timeout_expired_ = false;
    while ((cli->received_answer_ == false) && (cli->timeout_expired_ == false))
    {
        mgr.fd_poll();
        mgr.timer_check_exp();
    }

    int return_status = (cli->received_answer_ == true) ? 0 : 1;

    //
    // Destroy
    //
    mgr.block_stop(1);
    mgr.block_stop(2);

    mgr.block_del(1);
    mgr.block_del(2);

    mgr.block_factory_unregister("ncli");
    mgr.block_factory_unregister("hook_zmq");

    LOGGER_CLOSE();

    return return_status;
}
