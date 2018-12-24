

#include "engine/block.hpp"
#include "engine/manager.hpp"

//
// @brief Block file descriptor constructor
//
file_desc::file_desc() : bk(nullptr), fd(-1), socket(nullptr), read(false), write(false) {}

//
// @brief Block constructor and destructor
//
block::block(struct manager *mgr) : id_(0), state_(STATE_STOP), mgr_(mgr) {}
block::~block() {}

// Block interface default implementation
void block::bind_(int, int) {}
void block::start_() {}
void block::stop_() {}
int block::data_(void *) { return 0; }
void block::ctrl_(void *) {}
void block::on_timer_(struct timer &) {}
void block::on_fd_(struct file_desc &) {}

//
// @brief Send NOTIF data to a block
//
void block::process_ctrl_(int bk_id, void *notif)
{
    struct block *bk;

    bk = mgr_->block_get(bk_id);
    if (bk == nullptr)
    {
        LOGGER_ERR("Failed to notify block: unknown block [bk_id=%d]", bk_id);
        return;
    }
    bk->ctrl_(notif);
}

//
// @brief Start a data flow from this block
//
// @param port  : Source port to find a peer block
// @param data  : Data to process
// @param type  : Flow type (RX, TX)
//
void block::process_data_(int port, void *data)
{
    struct block *current;

    // Process the data from one block to the other
    current = this;
    while (true)
    {
        const struct bind_info *bind;

        if (port == PORT_STOP)
        {
            LOGGER_DEBUG("Ended data flow [bk_id_begin=%d ; bk_id_end=%d]", id_, current->id_);
            break;
        }

        // Find the source port in bindings
        bind = nullptr;
        for (const auto &it : current->binds_)
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
            LOGGER_ERR("Failed to route data flow: unknown port [bk_id=%d ; port=%d]", current->id_, port);
            return;
        }

        LOGGER_DEBUG("Routed data flow [bk_id_src=%d ; port=%d ; bk_id_dst=%d]", current->id_, port, bind->bk->id_);

        // The destination block is the new source of the data flow
        current = bind->bk;
        port = current->data_(data);
    }
}
