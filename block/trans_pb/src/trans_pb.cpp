

#define LOGGER_TAG "[block.trans_pb]"

// Project headers
#include "block/trans_pb.hpp"
#include "engine/manager.hpp"
#include "utils/socket.hpp"

// Generated protobuf command
#include "conf.pb-c.h"

trans_pb::trans_pb(struct manager *mgr) : block(mgr) {}
trans_pb::~trans_pb() {}

//
// @brief Parse and execute a protobuf command
//
void trans_pb::parse_command(const uint8_t *data, size_t size)
{
    Command *cmd;

    cmd = command__unpack(nullptr, size, data);
    if (cmd == nullptr)
    {
        LOGGER_ERR("Failed to unpack protobuf command: unknown reason [size=%zu]", size);
        return;
    }

    switch (cmd->type_case)
    {
    case COMMAND__TYPE_ADD:
        mgr_->block_add(cmd->add->id, cmd->add->type);
        break;

    case COMMAND__TYPE_START:
        mgr_->block_start(cmd->start->id);
        break;

    case COMMAND__TYPE_STOP:
        mgr_->block_stop(cmd->stop->id);
        break;

    case COMMAND__TYPE_DEL:
        mgr_->block_del(cmd->del->id);
        break;

    case COMMAND__TYPE_CONF:
        mgr_->block_conf(cmd->conf->id, cmd->conf->conf);
        break;

    case COMMAND__TYPE_BIND:
        mgr_->block_bind(cmd->bind->id, cmd->bind->port, cmd->bind->dest);
        break;

    case COMMAND__TYPE__NOT_SET:
    default:
        LOGGER_ERR("Failed to execute protobuf command: unknown command type [type=%d]", cmd->type_case);
        break;
    }

    command__free_unpacked(cmd, nullptr);
}

int trans_pb::rx_(void *vdata)
{
    if (vdata == nullptr)
    {
        LOGGER_ERR("trans_pb RX failed: nullptr data");
        return 0;
    }

    std::vector<struct c3qo_zmq_part> &msg = *(static_cast<std::vector<struct c3qo_zmq_part> *>(vdata));
    if (msg.size() != 2u)
    {
        LOGGER_ERR("Failed to decode message: unexpected message parts count [expected=%u ; actual=%zu]",
                   2u, msg.size());
        return 0;
    }

    // Action to take upon topic value
    if (strcmp("CONF.PROTO.CMD", msg[0].data) == 0)
    {
        parse_command(reinterpret_cast<uint8_t *>(msg[1].data), msg[1].len);
    }
    else
    {
        LOGGER_ERR("Failed to decode message: unknown topic [topic=%s]", msg[0].data);
        return 0;
    }

    return 0;
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
