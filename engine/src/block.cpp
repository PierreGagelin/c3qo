
#define LOGGER_TAG "[block.lib]"

#include "engine/block.hpp"

//
// @brief Stringify the block command
//
const char *bk_cmd_to_string(enum bk_cmd t)
{
    switch (t)
    {
    case CMD_ADD:
        return "BLOCK_COMMAND_ADD";

    case CMD_START:
        return "BLOCK_COMMAND_START";

    case CMD_STOP:
        return "BLOCK_COMMAND_STOP";

    case CMD_DEL:
        return "BLOCK_COMMAND_DEL";

    case CMD_CONF:
        return "BLOCK_COMMAND_CONF";

    case CMD_BIND:
        return "BLOCK_COMMAND_BIND";

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

    case STATE_START:
        return "BLOCK_STATE_STARTED";

    default:
        return "BLOCK_STATE_UNKNOWN";
    }
}

//
// @brief Convert flow type into a string
//
const char *flow_type_to_string(enum flow_type type)
{
    switch (type)
    {
    case FLOW_NOTIF:
        return "FLOW_TYPE_NOTIF";

    case FLOW_RX:
        return "FLOW_TYPE_RX";

    case FLOW_TX:
        return "FLOW_TYPE_TX";

    default:
        return "FLOW_TYPE_UNKNOWN";
    }
}

//
// @brief Block file descriptor constructor
//
file_desc::file_desc() : bk(nullptr), fd(-1), socket(nullptr), read(false), write(false) {}

//
// @brief Block constructor and destructor
//
block::block(struct manager *mgr) : id_(0), state_(STATE_STOP), mgr_(mgr) {}
block::~block() {}

// Management interface default implementation
void block::conf_(char *) {}
void block::bind_(int, int) {}
void block::start_() {}
void block::stop_() {}
int block::rx_(void *) { return 0; }
int block::tx_(void *) { return 0; }
int block::ctrl_(void *) { return 0; }
void block::on_timer_(struct timer &) {}
void block::on_fd_(struct file_desc &) {}

//
// @brief Send RX data to a block
//
void block::process_rx_(int port, void *data)
{
    process_flow_(port, data, FLOW_RX);
}

//
// @brief Send TX data to a block
//
void block::process_tx_(int port, void *data)
{
    process_flow_(port, data, FLOW_TX);
}

//
// @brief Send NOTIF data to a block
//
void block::process_notif_(int port, void *notif)
{
    process_flow_(port, notif, FLOW_NOTIF);
}

//
// @brief Start a data flow from this block
//
// @param port  : Source port to find a peer block
// @param data  : Data to process
// @param type  : Flow type (RX, TX, NOTIF)
//
void block::process_flow_(int port, void *data, enum flow_type type)
{
    if (state_ != STATE_START)
    {
        LOGGER_WARNING("Cannot start data flow: block is not started [bk_id=%d ; bk_state=%s ; flow_type=%s]",
                       id_, bk_state_to_string(state_), flow_type_to_string(type));
        return;
    }

    // Get a copy of the source that will serve to iterate over the blocks
    struct block *src = this;

    // Process the data from one block to the other
    while (true)
    {
        // Find the source port in bindings
        const struct bind_info *bind = nullptr;
        for (const auto &it : src->binds_)
        {
            if (it.port == port)
            {
                // We found the requested port
                bind = &it;
                break;
            }
        }
        if (bind == nullptr)
        {
            LOGGER_ERR("Failed to find route for data flow: source port not found [bk_id=%d ; port=%d]", src->id_, port);
            return;
        }

        LOGGER_DEBUG("Routed data flow [bk_id_src=%d ; port=%d ; bk_id_dest=%d]", src->id_, port, bind->bk_id);

        // Destination bk_id=0 is the regular way out of this flow
        if (bind->bk_id == 0)
        {
            LOGGER_DEBUG("End data flow [bk_id=%d ; data=%p ; flow_type=%s]", src->id_, data, flow_type_to_string(type));
            break;
        }

        // The destination block is the new source of the data flow
        src = bind->bk;

        switch (type)
        {
        case FLOW_NOTIF:
            LOGGER_DEBUG("Notify block [bk_id=%d ; notif=%p]", src->id_, data);
            port = src->ctrl_(data);
            break;

        case FLOW_RX:
            LOGGER_DEBUG("Process RX data [bk_id=%d ; data=%p]", src->id_, data);
            port = src->rx_(data);
            break;

        case FLOW_TX:
            LOGGER_DEBUG("Process TX data [bk_id=%d ; data=%p]", src->id_, data);
            port = src->tx_(data);
            break;

        default:
            LOGGER_ERR("Failed to flow data: unknown flow type [bk_id=%d ; data=%p ; flow_type=%d]",
                       src->id_, data, type);
            return;
        }
    }
}
