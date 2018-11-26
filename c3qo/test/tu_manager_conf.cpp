//
// @brief Test file for the block manager
//

// Project headers
#include "c3qo/tu.hpp"

// Generated protobuf command
#include "conf.pb-c.h"

// C++ headers
#include <fstream>
#include <sstream>

struct manager mgr_;

//
// @brief Send a protobuf command and return its result
//
bool proto_cmd_send(int block_id, PbcCmd__CmdType cmd_type, const char *arg, bool raw_dump = true)
{
    PbcCmd cmd;
    uint8_t *buffer;
    bool ret;

    // Prepare a protobuf message
    pbc_cmd__init(&cmd);
    cmd.block_arg = strdup(arg);
    cmd.has_block_id = 1;
    cmd.block_id = block_id;
    cmd.type = cmd_type;

    size_t size = pbc_cmd__get_packed_size(&cmd);
    buffer = new uint8_t[size + 1];

    pbc_cmd__pack(&cmd, buffer);
    buffer[size] = '\0';

    ret = mgr_.conf_parse_pb_cmd(buffer, size);

    // Dump a raw output of a command
    if (raw_dump == true)
    {
        const char *filename;

        switch (cmd_type)
        {
        case PBC_CMD__CMD_TYPE__CMD_ADD:
            filename = "/tmp/pbc_add.txt";
            break;
        case PBC_CMD__CMD_TYPE__CMD_START:
            filename = "/tmp/pbc_start.txt";
            break;
        case PBC_CMD__CMD_TYPE__CMD_STOP:
            filename = "/tmp/pbc_stop.txt";
            break;
        case PBC_CMD__CMD_TYPE__CMD_DEL:
            filename = "/tmp/pbc_del.txt";
            break;
        case PBC_CMD__CMD_TYPE__CMD_CONF:
            filename = "/tmp/pbc_conf.txt";
            break;
        case PBC_CMD__CMD_TYPE__CMD_BIND:
            filename = "/tmp/pbc_bind.txt";
            break;
        default:
            ASSERT(false);
        }

        FILE *out = fopen(filename, "w");
        ASSERT(out != nullptr);
        int fd = fileno(out);
        ASSERT(fd != -1);
        ssize_t ret = write(fd, buffer, size);
        ASSERT(static_cast<size_t>(ret) == size);
        int close = fclose(out);
        ASSERT(close == 0);
    }

    free(cmd.block_arg);
    delete[] buffer;

    return ret;
}

//
// @brief Test the configuration
//
static void tu_manager_conf_conf()
{
    char fname[] = "/tmp/tu_manager_conf.txt";
    std::fstream file;
    char buf[512];
    std::string buf_exp;
    std::stringstream ss;
    size_t len;

    file.open(fname, std::ios::out | std::ios::trunc);
    ASSERT(file.is_open() == true);

    // Add, initialize, configure and start 2 blocks hello
    // Spaces should not matter, but 3 values are mandatory
    file << CMD_ADD << "   1    hello        " << std::endl;
    file << CMD_ADD << "   2    hello        " << std::endl;

    file << CMD_CONF << "  1  hello_1 " << std::endl;
    file << CMD_CONF << "  2  hello_2 " << std::endl;

    file << CMD_START << " 1 " << std::endl;
    file << CMD_START << " 2 " << std::endl;

    // Bindings for block 1:
    //   - port=0 ; bk_id=2
    //   - port=2 ; bk_id=2
    //   - port=4 ; bk_id=2
    //   - port=6 ; bk_id=2
    file << CMD_BIND << " 1  0:2 " << std::endl;
    file << CMD_BIND << " 1  2:2 " << std::endl;
    file << CMD_BIND << " 1  4:2 " << std::endl;
    file << CMD_BIND << " 1  6:2 " << std::endl;

    file.close();

    // Parsing configuration
    ASSERT(mgr_.conf_parse(fname) == true);

    // Prepare expected configuration dump for the blocks
    //   - format : "<bk_id> <bk_type> <bk_state>;"
    ss << "1 hello " << STATE_START << ";";
    ss << "2 hello " << STATE_START << ";";
    buf_exp = ss.str();

    // Verify the configuration dump
    len = mgr_.conf_get(buf, sizeof(buf));
    ASSERT(len == buf_exp.length());

    // Verify block informations
    for (int i = 1; i < 3; i++)
    {
        const struct block *bi;

        bi = mgr_.block_get(i);
        ASSERT(bi != nullptr);

        ASSERT(bi->id_ == i);
        ASSERT(bi->state_ == STATE_START);
        ASSERT(bi->type_ == "hello");
    }

    // Clean blocks
    mgr_.block_clear();
}

//
// @brief Edge cases
//
static void tu_manager_conf_errors()
{
    char fname[] = "/tmp/tu_manager_conf.txt";
    std::fstream file;

    // Do not display error messages as we known there will be
    logger_set_level(LOGGER_LEVEL_CRIT);

    for (int i = 0; i < 8; i++)
    {
        file.open(fname, std::ios::out | std::ios::trunc);
        ASSERT(file.is_open() == true);

        switch (i)
        {
        case 0:
            // Undefined block ID
            file << CMD_ADD << " 0 hello" << std::endl;
            break;

        case 1:
            // Undefined block type
            file << CMD_ADD << " 1 " << 42 << std::endl;
            break;

        case 2:
            // Delete an unexisting block
            file << CMD_DEL << " 4 no_arg" << std::endl;
            break;

        case 3:
            // Configure an unexisting block
            file << CMD_CONF << " 4 hello_1" << std::endl;
            break;

        case 4:
            // Bind an unexisting block
            file << CMD_BIND << " 4  0:2" << std::endl;
            break;

        case 5:
            // Start an unexisting block
            file << CMD_START << " 4 no_arg" << std::endl;
            break;

        case 6:
            // Too many entries
            file << "I like to try out fancy entries in configuration" << std::endl;
            break;

        case 7:
            // Not enough entries
            file << "Both ways " << std::endl;
            break;

        default:
            // Incorrect test
            ASSERT(false);
            break;
        }

        file.close();
        ASSERT(mgr_.conf_parse(fname) == false);
    }

    ASSERT((proto_cmd_send(42, static_cast<PbcCmd__CmdType>(12), "", false)) == false);
}

static void tu_manager_conf_pbc_conf()
{
    struct block *bk;
    int bk_id = 42;

    // Add a block
    ASSERT(proto_cmd_send(bk_id, PBC_CMD__CMD_TYPE__CMD_ADD, "hello"));
    bk = mgr_.block_get(bk_id);
    ASSERT(bk != nullptr);

    ASSERT(proto_cmd_send(bk_id, PBC_CMD__CMD_TYPE__CMD_CONF, "my_name_is"));
    ASSERT(proto_cmd_send(bk_id, PBC_CMD__CMD_TYPE__CMD_BIND, "2:5"));

    ASSERT(proto_cmd_send(bk_id, PBC_CMD__CMD_TYPE__CMD_START, ""));
    ASSERT(bk->state_ == STATE_START);

    ASSERT(proto_cmd_send(bk_id, PBC_CMD__CMD_TYPE__CMD_STOP, ""));
    ASSERT(bk->state_ == STATE_STOP);

    ASSERT(proto_cmd_send(bk_id, PBC_CMD__CMD_TYPE__CMD_DEL, ""));
    ASSERT(mgr_.block_get(bk_id) == nullptr);
}

int main(int, char **)
{
    LOGGER_OPEN("tu_manager_conf");
    logger_set_level(LOGGER_LEVEL_DEBUG);

    tu_manager_conf_conf();
    tu_manager_conf_errors();
    tu_manager_conf_pbc_conf();

    LOGGER_CLOSE();
    return 0;
}
