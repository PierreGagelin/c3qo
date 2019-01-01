

// C headers
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

ncli::ncli(struct manager *mgr) : block(mgr),
                                  ncli_peer_(nullptr),
                                  ncli_cmd_type_(nullptr),
                                  ncli_cmd_args_(nullptr),
                                  ncli_cmd_ret_(nullptr),
                                  add_id_(0),
                                  add_type_(nullptr),
                                  start_id_(0),
                                  stop_id_(0),
                                  del_id_(0),
                                  bind_id_(0),
                                  bind_port_(0),
                                  bind_dest_(0),
                                  hook_zmq_id_(0),
                                  hook_zmq_client_(false),
                                  hook_zmq_type_(0),
                                  hook_zmq_name_(nullptr),
                                  hook_zmq_addr_(nullptr)
{
}
ncli::~ncli() {}

bool ncli::parse_add(int argc, char **argv)
{
    const char *options = "i:t:";
    for (int opt = getopt(argc, argv, options); opt != -1; opt = getopt(argc, argv, options))
    {
        switch (opt)
        {
        case 'i':
            LOGGER_DEBUG("Set block identifier [value=%s]", optarg);
            add_id_ = atoi(optarg);
            break;

        case 't':
            LOGGER_DEBUG("Set block type [value=%s]", optarg);
            add_type_ = optarg;
            break;

        default:
            LOGGER_ERR("Failed to parse option: unknown option [opt=%c]", static_cast<char>(opt));
            return false;
        }
    }

    command__init(&cmd_);
    cmd_.type_case = COMMAND__TYPE_ADD;
    block_add__init(&bk_add_);
    cmd_.add = &bk_add_;
    cmd_.add->id = add_id_;
    cmd_.add->type = add_type_;

    return true;
}

bool ncli::parse_start(int argc, char **argv)
{
    const char *options = "i:";
    for (int opt = getopt(argc, argv, options); opt != -1; opt = getopt(argc, argv, options))
    {
        switch (opt)
        {
        case 'i':
            LOGGER_DEBUG("Set block identifier [value=%s]", optarg);
            start_id_ = atoi(optarg);
            break;

        default:
            LOGGER_ERR("Failed to parse option: unknown option [opt=%c]", static_cast<char>(opt));
            return false;
        }
    }

    command__init(&cmd_);
    cmd_.type_case = COMMAND__TYPE_START;
    block_start__init(&bk_start_);
    cmd_.start = &bk_start_;
    cmd_.start->id = start_id_;

    return true;
}

bool ncli::parse_stop(int argc, char **argv)
{
    const char *options = "i:";
    for (int opt = getopt(argc, argv, options); opt != -1; opt = getopt(argc, argv, options))
    {
        switch (opt)
        {
        case 'i':
            LOGGER_DEBUG("Set block identifier [value=%s]", optarg);
            stop_id_ = atoi(optarg);
            break;

        default:
            LOGGER_ERR("Failed to parse option: unknown option [opt=%c]", static_cast<char>(opt));
            return false;
        }
    }

    command__init(&cmd_);
    cmd_.type_case = COMMAND__TYPE_STOP;
    block_stop__init(&bk_stop_);
    cmd_.stop = &bk_stop_;
    cmd_.stop->id = stop_id_;

    return true;
}

bool ncli::parse_del(int argc, char **argv)
{
    const char *options = "i:";
    for (int opt = getopt(argc, argv, options); opt != -1; opt = getopt(argc, argv, options))
    {
        switch (opt)
        {
        case 'i':
            LOGGER_DEBUG("Set block identifier [value=%s]", optarg);
            del_id_ = atoi(optarg);
            break;

        default:
            LOGGER_ERR("Failed to parse option: unknown option [opt=%c]", static_cast<char>(opt));
            return false;
        }
    }

    command__init(&cmd_);
    cmd_.type_case = COMMAND__TYPE_DEL;
    block_del__init(&bk_del_);
    cmd_.del = &bk_del_;
    cmd_.del->id = del_id_;

    return true;
}

bool ncli::parse_bind(int argc, char **argv)
{
    const char *options = "i:p:d:";
    for (int opt = getopt(argc, argv, options); opt != -1; opt = getopt(argc, argv, options))
    {
        switch (opt)
        {
        case 'i':
            LOGGER_DEBUG("Set bind source [value=%s]", optarg);
            bind_id_ = atoi(optarg);
            break;

        case 'p':
            LOGGER_DEBUG("Set bind port [value=%s]", optarg);
            bind_port_ = atoi(optarg);
            break;

        case 'd':
            LOGGER_DEBUG("Set bind destination [value=%s]", optarg);
            bind_dest_ = atoi(optarg);
            break;

        default:
            LOGGER_ERR("Failed to parse option: unknown option [opt=%c]", static_cast<char>(opt));
            return false;
        }
    }

    command__init(&cmd_);
    cmd_.type_case = COMMAND__TYPE_BIND;
    block_bind__init(&bk_bind_);
    cmd_.bind = &bk_bind_;
    cmd_.bind->id = bind_id_;
    cmd_.bind->port = bind_port_;
    cmd_.bind->dest = bind_dest_;

    return true;
}

bool ncli::parse_hook_zmq(int argc, char **argv)
{
    const char *options = "i:ct:n:a:";
    for (int opt = getopt(argc, argv, options); opt != -1; opt = getopt(argc, argv, options))
    {
        switch (opt)
        {
        case 'i':
            LOGGER_DEBUG("Set hook identifier [value=%s]", optarg);
            hook_zmq_id_ = atoi(optarg);
            break;

        case 'c':
            LOGGER_DEBUG("Set hook client");
            hook_zmq_client_ = true;
            break;

        case 't':
            LOGGER_DEBUG("Set hook type [value=%s]", optarg);
            hook_zmq_type_ = atoi(optarg);
            break;

        case 'n':
            LOGGER_DEBUG("Set hook name [value=%s]", optarg);
            hook_zmq_name_ = optarg;
            break;

        case 'a':
            LOGGER_DEBUG("Set hook address [value=%s]", optarg);
            hook_zmq_addr_ = optarg;
            break;

        default:
            LOGGER_ERR("Failed to parse option: unknown option [opt=%c]", static_cast<char>(opt));
            return false;
        }
    }

    command__init(&cmd_);
    cmd_.type_case = COMMAND__TYPE_HOOK_ZMQ;
    conf_hook_zmq__init(&conf_hook_zmq_);
    cmd_.hook_zmq = &conf_hook_zmq_;
    cmd_.hook_zmq->id = hook_zmq_id_;
    cmd_.hook_zmq->client = hook_zmq_client_;
    cmd_.hook_zmq->type = hook_zmq_type_;
    cmd_.hook_zmq->name = hook_zmq_name_;
    cmd_.hook_zmq->addr = hook_zmq_addr_;

    return true;
}

bool ncli::parse_term(int, char **)
{
    command__init(&cmd_);
    cmd_.type_case = COMMAND__TYPE_TERM;
    cmd_.term = true;

    return true;
}

bool ncli::options_parse(int argc, char **argv)
{
    const char *options = "i:o:t:r:";
    for (int opt = getopt(argc, argv, options); opt != -1; opt = getopt(argc, argv, options))
    {
        switch (opt)
        {
        case 'i':
            LOGGER_DEBUG("Set peer identity [value=%s]", optarg);
            ncli_peer_ = optarg;
            break;

        case 'o':
            LOGGER_DEBUG("Set command options [value=%s]", optarg);
            ncli_cmd_args_ = optarg;
            break;

        case 't':
            LOGGER_DEBUG("Set command type [value=%s]", optarg);
            ncli_cmd_type_ = optarg;
            break;

        case 'r':
            LOGGER_DEBUG("Set command expected return [value=%s]", optarg);
            ncli_cmd_ret_ = optarg;
            break;

        default:
            LOGGER_ERR("Failed to parse option: unknown option [opt=%c]", static_cast<char>(opt));
            return false;
        }
    }
    ASSERT(ncli_peer_ != nullptr);
    ASSERT(ncli_cmd_type_ != nullptr);
    ASSERT(ncli_cmd_args_ != nullptr);
    ASSERT(ncli_cmd_ret_ != nullptr);

    ASSERT(wordexp(ncli_cmd_args_, &wordexp_, 0) == 0);
    optind = 1; // reset getopt

    bool ret;
    if (strcmp(ncli_cmd_type_, "add") == 0)
    {
        ret = parse_add(wordexp_.we_wordc, wordexp_.we_wordv);
    }
    else if (strcmp(ncli_cmd_type_, "start") == 0)
    {
        ret = parse_start(wordexp_.we_wordc, wordexp_.we_wordv);
    }
    else if (strcmp(ncli_cmd_type_, "stop") == 0)
    {
        ret = parse_stop(wordexp_.we_wordc, wordexp_.we_wordv);
    }
    else if (strcmp(ncli_cmd_type_, "del") == 0)
    {
        ret = parse_del(wordexp_.we_wordc, wordexp_.we_wordv);
    }
    else if (strcmp(ncli_cmd_type_, "bind") == 0)
    {
        ret = parse_bind(wordexp_.we_wordc, wordexp_.we_wordv);
    }
    else if (strcmp(ncli_cmd_type_, "hook_zmq") == 0)
    {
        ret = parse_hook_zmq(wordexp_.we_wordc, wordexp_.we_wordv);
    }
    else if (strcmp(ncli_cmd_type_, "term") == 0)
    {
        ret = parse_term(wordexp_.we_wordc, wordexp_.we_wordv);
    }
    else
    {
        LOGGER_ERR("Failed to parse option: unknown command type [type=%s]", ncli_cmd_type_);
        ret = false;
    }

    return ret;
}

void ncli::options_clear()
{
    wordfree(&wordexp_);
}

void ncli::start_()
{
    //
    // Prepare and send the network command
    //
    struct buffer buf;

    buf.push_back(ncli_peer_, strlen(ncli_peer_) + 1);

    size_t size = command__get_packed_size(&cmd_);
    uint8_t *proto = new uint8_t[size];
    command__pack(&cmd_, proto);

    // Fill ZMQ message
    const char *topic = "PROTO.CMD";
    buf.push_back(topic, strlen(topic));

    buf.push_back(proto, size);

    process_data_(&buf);

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

    // Release command options
    options_clear();
}

bool ncli::data_(void *vdata)
{
    struct buffer &buf = *(static_cast<struct buffer *>(vdata));

    if (buf.parts_.size() != 3u)
    {
        LOGGER_DEBUG("Discard message: wrong parts count [expected=3 ; actual=%zu]", buf.parts_.size());
        return false;
    }
    if ((memcmp(buf.parts_[0].data, ncli_peer_, buf.parts_[0].len) != 0) ||
        (memcmp(buf.parts_[1].data, "PROTO.CMD.REP", buf.parts_[1].len) != 0) ||
        (memcmp(buf.parts_[2].data, ncli_cmd_ret_, buf.parts_[2].len) != 0))
    {
        LOGGER_DEBUG("Discard unexpected message [peer=%s ; topic=%s ; status=%s]",
                     static_cast<char *>(buf.parts_[0].data),
                     static_cast<char *>(buf.parts_[1].data),
                     static_cast<char *>(buf.parts_[2].data));
        return false;
    }

    LOGGER_INFO("Received expected answer");
    received_answer_ = true;

    return false;
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

    ASSERT(cli->options_parse(argc, argv) == true);

    //
    // Bind and start
    //
    mgr.block_bind(1, 0, 2);
    mgr.block_bind(2, 0, 1);

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
