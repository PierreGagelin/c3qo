//
// @brief Test file for the block manager
//

// Project headers
#include "engine/tu.hpp"

// Generated protobuf command
#include "conf.pb-c.h"

struct manager mgr_;

//
// @brief Send a protobuf command and return its result
//
static bool proto_cmd_send(Command__TypeCase type,
                           int block_id,
                           const char *arg,
                           int port = 0,
                           int dest = 0)
{
    char *block_arg;
    bool ret;

    block_arg = strdup(arg);

    // Prepare a protobuf message
    Command cmd;
    BlockAdd add;
    BlockStart start;
    BlockStop stop;
    BlockDel del;
    BlockConf conf;
    BlockBind bind;

    command__init(&cmd);

    switch (type)
    {
    case COMMAND__TYPE_ADD:
        cmd.type_case = COMMAND__TYPE_ADD;
        block_add__init(&add);
        cmd.add = &add;
        cmd.add->id = block_id;
        cmd.add->type = block_arg;
        break;

    case COMMAND__TYPE_START:
        cmd.type_case = COMMAND__TYPE_START;
        block_start__init(&start);
        cmd.start = &start;
        cmd.start->id = block_id;
        break;

    case COMMAND__TYPE_STOP:
        cmd.type_case = COMMAND__TYPE_STOP;
        block_stop__init(&stop);
        cmd.stop = &stop;
        cmd.stop->id = block_id;
        break;

    case COMMAND__TYPE_DEL:
        cmd.type_case = COMMAND__TYPE_DEL;
        block_del__init(&del);
        cmd.del = &del;
        cmd.del->id = block_id;
        break;

    case COMMAND__TYPE_CONF:
        cmd.type_case = COMMAND__TYPE_CONF;
        block_conf__init(&conf);
        cmd.conf = &conf;
        cmd.conf->id = block_id;
        cmd.conf->conf = block_arg;
        break;

    case COMMAND__TYPE_BIND:
        cmd.type_case = COMMAND__TYPE_BIND;
        block_bind__init(&bind);
        cmd.bind = &bind;
        cmd.bind->id = block_id;
        cmd.bind->port = port;
        cmd.bind->dest = dest;
        break;

    case COMMAND__TYPE__NOT_SET:
    default:
        cmd.type_case = COMMAND__TYPE__NOT_SET;
        break;
    }

    size_t size = command__get_packed_size(&cmd);
    uint8_t *buffer = new uint8_t[size + 1];

    command__pack(&cmd, buffer);
    buffer[size] = '\0';

    ret = mgr_.conf_parse_pb_cmd(buffer, size);

    free(block_arg);
    delete[] buffer;

    return ret;
}

//
// @brief Edge cases
//
static void tu_manager_conf_errors()
{
    // Do not display error messages as we known there will be
    logger_set_level(LOGGER_LEVEL_CRIT);

    ASSERT((proto_cmd_send(static_cast<Command__TypeCase>(12), 42, "")) == false);

    logger_set_level(LOGGER_LEVEL_DEBUG);
}

static void tu_manager_conf_pbc_conf()
{
    struct block *bk;
    int bk_id = 42;

    // Add a block
    ASSERT(proto_cmd_send(COMMAND__TYPE_ADD, bk_id, "trans_pb"));
    bk = mgr_.block_get(bk_id);
    ASSERT(bk != nullptr);

    ASSERT(proto_cmd_send(COMMAND__TYPE_CONF, bk_id, "my_name_is"));
    ASSERT(proto_cmd_send(COMMAND__TYPE_BIND, bk_id, "", 2, 5));

    ASSERT(proto_cmd_send(COMMAND__TYPE_START, bk_id, ""));
    ASSERT(bk->state_ == STATE_START);

    ASSERT(proto_cmd_send(COMMAND__TYPE_STOP, bk_id, ""));
    ASSERT(bk->state_ == STATE_STOP);

    ASSERT(proto_cmd_send(COMMAND__TYPE_DEL, bk_id, ""));
    ASSERT(mgr_.block_get(bk_id) == nullptr);
}

int main(int, char **)
{
    LOGGER_OPEN("tu_manager_conf");
    logger_set_level(LOGGER_LEVEL_DEBUG);

    tu_manager_conf_errors();
    tu_manager_conf_pbc_conf();

    LOGGER_CLOSE();
    return 0;
}
