

// Project headers
#include "block/trans_pb.hpp"
#include "engine/manager.hpp"
#include "utils/socket.hpp"

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

    case COMMAND__TYPE_CONF:
        is_ok = mgr_->block_conf(cmd->conf->id, cmd->conf->conf);
        break;

    case COMMAND__TYPE_BIND:
        is_ok = mgr_->block_bind(cmd->bind->id, cmd->bind->port, cmd->bind->dest);
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
    std::vector<struct c3qo_zmq_part> msg;
    struct c3qo_zmq_part part;
    const char *topic;
    const char *status;

    topic = "CONF.PROTO.CMD.REP";
    part.len = strlen(topic);
    part.data = new char[part.len];
    memcpy(part.data, topic, part.len);
    msg.push_back(part);

    status = is_ok ? "OK" : "KO";
    part.len = strlen(status);
    part.data = new char[part.len];
    memcpy(part.data, status, part.len);
    msg.push_back(part);

    process_data_(1, &msg);

    c3qo_zmq_msg_del(msg);
}

int trans_pb::data_(void *vdata)
{
    if (vdata == nullptr)
    {
        LOGGER_ERR("Failed to process data: nullptr data");
        return PORT_STOP;
    }

    std::vector<struct c3qo_zmq_part> &msg = *(static_cast<std::vector<struct c3qo_zmq_part> *>(vdata));
    if (msg.size() != 2u)
    {
        LOGGER_ERR("Failed to decode message: unexpected parts count [expected=%u ; actual=%zu]", 2u, msg.size());
        return PORT_STOP;
    }

    // Action to take upon topic value
    if (strcmp("CONF.PROTO.CMD", msg[0].data) == 0)
    {
        bool is_ok;
        is_ok = proto_command_parse(reinterpret_cast<uint8_t *>(msg[1].data), msg[1].len);
        proto_command_reply(is_ok);
    }
    else
    {
        LOGGER_ERR("Failed to decode message: unknown topic [topic=%s]", msg[0].data);
        return PORT_STOP;
    }

    return PORT_STOP;
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
