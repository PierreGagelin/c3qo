

#include "c3qo/block.hpp"

//
// @brief Stringify the block type
//
const char *get_bk_type(enum bk_type t)
{
    switch (t)
    {
    case TYPE_HELLO:
        return "BLOCK_TYPE_HELLO";

    case TYPE_CLIENT_US_NB:
        return "BLOCK_TYPE_CLIENT_US_NB";

    case TYPE_SERVER_US_NB:
        return "BLOCK_TYPE_SERVER_US_NB";

    case TYPE_PUB_SUB:
        return "BLOCK_TYPE_PUB_SUB";

    default:
        return "BLOCK_TYPE_UNKNOWN";
    }
}

//
// @brief Stringify the block command
//
const char *get_bk_cmd(enum bk_cmd t)
{
    switch (t)
    {
    case CMD_ADD:
        return "BLOCK_COMMAND_ADD";

    case CMD_INIT:
        return "BLOCK_COMMAND_INIT";

    case CMD_CONF:
        return "BLOCK_COMMAND_CONF";

    case CMD_BIND:
        return "BLOCK_COMMAND_BIND";

    case CMD_START:
        return "BLOCK_COMMAND_START";

    case CMD_STOP:
        return "BLOCK_COMMAND_STOP";

    case CMD_STATS:
        return "BLOCK_COMMAND_STATS";

    default:
        return "BLOCK_COMMAND_UNKOWN";
    }
}

//
// @brief Stringify the block state
//
const char *get_bk_state(enum bk_state t)
{
    switch (t)
    {
    case STATE_STOP:
        return "BLOCK_STATE_STOPPED";

    case STATE_INIT:
        return "BLOCK_STATE_INITIALIZED";

    case STATE_START:
        return "BLOCK_STATE_STARTED";

    default:
        return "BLOCK_STATE_UNKNOWN";
    }
}
