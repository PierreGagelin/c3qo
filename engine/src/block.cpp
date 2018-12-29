

// Project headers
#include "engine/block.hpp"
#include "engine/manager.hpp"

//
// @brief Block constructor and destructor
//
block::block(struct manager *mgr) : id_(0), is_started_(false), sink_(nullptr), mgr_(mgr) {}
block::~block() {}

// Block interface default implementation
void block::bind_(int, struct block *) {}
void block::start_() {}
void block::stop_() {}
bool block::data_(void *) { return false; }
void block::ctrl_(void *) {}
void block::on_timer_(struct timer &) {}
void block::on_fd_(struct file_desc &) {}

//
// @brief Send a notification to a block
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
void block::process_data_(void *data)
{
    struct block *current;

    LOGGER_DEBUG("Started data flow [bk_id_src=%d]", id_);

    // Process the data from one block to the other
    current = this;
    while (true)
    {
        // Get the sink in which to send data
        current = current->sink_;
        if (current == nullptr)
        {
            LOGGER_ERR("Failed to forward data flow: no block bound");
            return;
        }

        LOGGER_DEBUG("Forwarding data [bk_id=%d]", current->id_);

        // The destination block is the new source of the data flow
        bool forward = current->data_(data);
        if (forward == false)
        {
            LOGGER_DEBUG("Stopped data flow [bk_id_src=%d ; bk_id_sink=%d]", id_, current->id_);
            break;
        }
    }
}
