

#include "c3qo/block.hpp"

extern struct bk_if hello_if;
extern struct bk_if client_us_nb_if;
extern struct bk_if server_us_nb_if;
extern struct bk_if zmq_pair_if;
extern struct bk_if project_euler_if;
extern struct bk_if trans_pb_if;

//
// @brief Retrieve a block interface pointer
//
struct bk_if *get_bk_if(const char *b)
{
    if (strcmp(b, "hello_if") == 0)
    {
        return &hello_if;
    }
    if (strcmp(b, "client_us_nb_if") == 0)
    {
        return &client_us_nb_if;
    }
    if (strcmp(b, "server_us_nb_if") == 0)
    {
        return &server_us_nb_if;
    }
    if (strcmp(b, "zmq_pair_if") == 0)
    {
        return &zmq_pair_if;
    }
    if (strcmp(b, "project_euler_if") == 0)
    {
        return &project_euler_if;
    }
    if (strcmp(b, "trans_pb_if") == 0)
    {
        return &trans_pb_if;
    }
    return NULL;
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
