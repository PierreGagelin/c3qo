

// Project headers
#include "block/hook_zmq.hpp"
#include "block/trans_pb.hpp"
#include "engine/manager.hpp"
#include "utils/buffer.hpp"

// Generated protobuf command
#include "conf.pb-c.h"

trans_pb::trans_pb(struct manager *mgr) : block(mgr) {}
trans_pb::~trans_pb() {}

//
// @brief Parse and execute a protobuf configuration command
//
bool trans_pb::proto_command_parse(const uint8_t *data, size_t size)
{
    Command *cmd;
    bool is_ok;

    cmd = command__unpack(nullptr, size, data);
    if (cmd == nullptr)
    {
        LOGGER_ERR("Failed to unpack protobuf command: unknown reason [size=%zu]", size);
        return false;
    }

    switch (cmd->type_case)
    {
    case COMMAND__TYPE_ADD:
        is_ok = mgr_->block_add(cmd->add->id, cmd->add->type);
        break;

    case COMMAND__TYPE_START:
        is_ok = mgr_->block_start(cmd->start->id);
        break;

    case COMMAND__TYPE_STOP:
        is_ok = mgr_->block_stop(cmd->stop->id);
        break;

    case COMMAND__TYPE_DEL:
        is_ok = mgr_->block_del(cmd->del->id);
        break;

    case COMMAND__TYPE_BIND:
        is_ok = mgr_->block_bind(cmd->bind->id, cmd->bind->port, cmd->bind->dest);
        break;

    case COMMAND__TYPE_HOOK_ZMQ:
    {
        struct hook_zmq *hook;
        hook = static_cast<struct hook_zmq *>(mgr_->block_get(cmd->hook_zmq->id));
        if (hook == nullptr)
        {
            LOGGER_ERR("Failed to configure ZMQ hook: unknown block [bk_id=%d]", cmd->hook_zmq->id);
            is_ok = false;
        }
        else
        {
            hook->client_ = cmd->hook_zmq->client;
            hook->type_ = cmd->hook_zmq->type;
            hook->name_ = std::string(cmd->hook_zmq->name);
            hook->addr_ = std::string(cmd->hook_zmq->addr);

            LOGGER_INFO("Configured ZMQ hook [bk_id=%d ; client=%s ; type=%d ; name=%s ; addr=%s]",
                        hook->id_,
                        hook->client_ ? "true" : "false",
                        hook->type_,
                        hook->name_.c_str(),
                        hook->addr_.c_str());
            is_ok = true;
        }
    }
    break;

    case COMMAND__TYPE_TERM:
        is_ok = true;
        mgr_->stop_();
        break;

    case COMMAND__TYPE__NOT_SET:
    default:
        LOGGER_ERR("Failed to execute protobuf command: unknown command type [type=%d]", cmd->type_case);
        is_ok = false;
        break;
    }

    command__free_unpacked(cmd, nullptr);

    return is_ok;
}

//
// @brief Reply to a protobuf configuration command
//
void trans_pb::proto_command_reply(bool is_ok)
{
    struct buffer buf;
    const char *topic;
    const char *status;

    topic = "PROTO.CMD.REP";
    buf.push_back(topic, strlen(topic));

    status = is_ok ? "OK" : "KO";
    buf.push_back(status, strlen(status));

    process_data_(&buf);

    buf.clear();
}

bool trans_pb::data_(void *vdata)
{
    if (vdata == nullptr)
    {
        LOGGER_ERR("Failed to process data: nullptr data");
        return false;
    }

    struct buffer &buf = *(static_cast<struct buffer *>(vdata));
    if (buf.parts_.size() != 2u)
    {
        LOGGER_ERR("Failed to decode message: unexpected parts count [expected=%u ; actual=%zu]", 2u, buf.parts_.size());
        return false;
    }

    // Action to take upon topic value
    if (memcmp("PROTO.CMD", buf.parts_[0].data, buf.parts_[0].len) == 0)
    {
        bool is_ok;
        is_ok = proto_command_parse(static_cast<uint8_t *>(buf.parts_[1].data), buf.parts_[1].len);
        proto_command_reply(is_ok);
    }
    else
    {
        LOGGER_ERR("Failed to decode message: unknown topic [topic=%s]",
                   static_cast<char *>(buf.parts_[0].data));
        return false;
    }

    return false;
}

//
// Implementation of the factory interface
//

struct block *trans_pb_factory::constructor(struct manager *mgr)
{
    return new struct trans_pb(mgr);
}

void trans_pb_factory::destructor(struct block *bk)
{
    delete static_cast<struct trans_pb *>(bk);
}
