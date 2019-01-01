//
// @brief Test file for a block
//

// Project headers
#include "block/trans_pb.hpp"
#include "engine/tu.hpp"

#include "conf.pb-c.h"

struct tu_trans_pb
{
    struct manager mgr_;
    struct trans_pb block_;
    struct trans_pb_factory block_factory_;

    tu_trans_pb() : block_(&mgr_)
    {
        mgr_.block_factory_register("trans_pb", &block_factory_);
    }

    bool proto_cmd_send(Command__TypeCase type, int block_id, const char *arg, int port = 0, int dest = 0)
    {
        char *block_arg;

        block_arg = strdup(arg);

        // Prepare a protobuf message
        Command cmd;
        BlockAdd add;
        BlockStart start;
        BlockStop stop;
        BlockDel del;
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

        struct buffer buf;

        const char *topic = "PROTO.CMD";
        buf.push_back(topic, strlen(topic));
        buf.push_back(buffer, size);

        block_.data_(&buf);

        buf.clear();

        free(block_arg);
        delete[] buffer;

        return true;
    }
};

//
// @brief Edge cases
//
static void tu_trans_pb_errors()
{
    struct tu_trans_pb test;

    // No message
    {
        test.block_.data_(nullptr);
    }

    // Message with wrong size
    {
        struct buffer buf;
        test.block_.data_(&buf);
    }

    // Message with unknown topic
    {
        struct buffer buf;

        const char *tmp = "what a nice topic you've got there";
        buf.push_back(tmp, strlen(tmp));
        buf.push_back(tmp, strlen(tmp));

        test.block_.data_(&buf);

        buf.clear();
    }

    // Message with bad protobuf data
    {
        struct buffer buf;

        const char *tmp = "PROTO.CMD";
        buf.push_back(tmp, strlen(tmp)); // topic: OK
        buf.push_back(tmp, strlen(tmp)); // protobuf data: not OK

        test.block_.data_(&buf);

        buf.clear();
    }

    // Unknown command type
    {
        test.proto_cmd_send(static_cast<Command__TypeCase>(12), 42, "");
    }
}

static void tu_trans_pb_pbc_conf()
{
    struct tu_trans_pb test;
    struct block *bk;
    int bk_id = 42;

    // Add a block
    ASSERT(test.proto_cmd_send(COMMAND__TYPE_ADD, bk_id, "trans_pb"));
    bk = test.mgr_.block_get(bk_id);
    ASSERT(bk != nullptr);

    ASSERT(test.proto_cmd_send(COMMAND__TYPE_BIND, bk_id, "", 2, 5));

    ASSERT(test.proto_cmd_send(COMMAND__TYPE_START, bk_id, ""));
    ASSERT(bk->is_started_ == true);

    ASSERT(test.proto_cmd_send(COMMAND__TYPE_STOP, bk_id, ""));
    ASSERT(bk->is_started_ == false);

    ASSERT(test.proto_cmd_send(COMMAND__TYPE_DEL, bk_id, ""));
    ASSERT(test.mgr_.block_get(bk_id) == nullptr);
}

int main(int, char **)
{
    LOGGER_OPEN("tu_trans_pb");

    tu_trans_pb_errors();
    tu_trans_pb_pbc_conf();

    LOGGER_CLOSE();
    return 0;
}
