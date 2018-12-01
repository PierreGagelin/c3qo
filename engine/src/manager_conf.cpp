

#define LOGGER_TAG "[engine.conf]"

// Project headers
#include "engine/manager.hpp"

// Generated protobuf command
#include "conf.pb-c.h"

//
// @brief Parse and execute a protobuf command
//
bool manager::conf_parse_pb_cmd(const uint8_t *pb_cmd, size_t size)
{
    Command *cmd;
    bool ret;

    cmd = command__unpack(nullptr, size, pb_cmd);
    if (cmd == nullptr)
    {
        LOGGER_ERR("Failed to unpack protobuf command: unknown reason [size=%zu]", size);
        return false;
    }

    switch (cmd->type_case)
    {
    case COMMAND__TYPE_ADD:
        ret = block_add(cmd->add->id, cmd->add->type);
        break;
    case COMMAND__TYPE_START:
        ret = block_start(cmd->start->id);
        break;
    case COMMAND__TYPE_STOP:
        ret = block_stop(cmd->stop->id);
        break;
    case COMMAND__TYPE_DEL:
        ret = block_del(cmd->del->id);
        break;
    case COMMAND__TYPE_CONF:
        ret = block_conf(cmd->conf->id, cmd->conf->conf);
        break;
    case COMMAND__TYPE_BIND:
        ret = block_bind(cmd->bind->id, cmd->bind->port, cmd->bind->dest);
        break;
    case COMMAND__TYPE__NOT_SET:
    default:
        LOGGER_ERR("Failed to execute protobuf command: unknown command type [type=%d]", cmd->type_case);
        goto err;
    }

    command__free_unpacked(cmd, nullptr);
    return ret;
err:
    command__free_unpacked(cmd, nullptr);
    return false;
}
