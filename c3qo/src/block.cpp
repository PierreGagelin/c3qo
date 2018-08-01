

#include "c3qo/block.hpp"

//
// @brief Stringify the block command
//
const char *bk_cmd_to_string(enum bk_cmd t)
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
const char *bk_state_to_string(enum bk_state t)
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

//
// @brief Block constructor and destructor
//
block::block() : ctx_(nullptr), id_(0), state_(STATE_STOP) {}
block::~block() {}

// Management interface default implementation
void block::init_() {}
void block::conf_(char *) {}
void block::bind_(int, int) {}
void block::start_() {}
void block::stop_() {}
size_t block::get_stats_(char *, size_t) { return 0u; }
int block::rx_(void *) { return 0; }
int block::tx_(void *) { return 0; }
int block::ctrl_(void *) { return 0; }
