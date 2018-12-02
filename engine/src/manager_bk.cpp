

#define LOGGER_TAG "[engine.block]"

// Project headers
#include "engine/manager.hpp"

//
// @brief Constructor and destructor
//
manager::manager()
{
    // Nothing to do
}
manager::~manager()
{
    block_clear();
    block_factory_clear();
}

//
// @brief Add or replace a block
//
// @param id   : Block ID
// @param type : Block type
//
bool manager::block_add(int id, const char *type)
{
    struct block *bk;

    // Check input
    if (id == 0)
    {
        LOGGER_ERR("Failed to add block: forbidden block ID [bk_id=%d ; bk_type=%s]", id, type);
        return false;
    }
    if (block_get(id) != nullptr)
    {
        LOGGER_ERR("Failed to add block: block already exists [bk_id=%d]", id);
        return false;
    }

    // Get the factory
    const auto &factory = bk_factory_.find(type);
    if (factory == bk_factory_.cend())
    {
        LOGGER_ERR("Failed to add block: no factory found [type=%s]", type);
        return false;
    }

    LOGGER_INFO("Add block [bk_id=%d ; bk_type=%s]", id, type);

    // Build a block
    bk = factory->second->constructor(this);
    if (bk == nullptr)
    {
        LOGGER_ERR("Failed to add block: construction failed [type=%s]", type);
        return false;
    }

    bk->id_ = id;
    bk->type_ = type;
    bk->state_ = STATE_STOP;

    bk_map_.insert({id, bk});

    return true;
}

//
// @brief Start a block
//
bool manager::block_start(int id)
{
    struct block *bk;

    bk = block_get(id);
    if (bk == nullptr)
    {
        LOGGER_WARNING("Cannot start block: unknown block ID [bk_id=%d]", id);
        return false;
    }

    // Verify block state
    if (bk->state_ == STATE_START)
    {
        // Nothing to do
        return true;
    }

    LOGGER_INFO("Start block [bk_id=%d ; bk_type=%s ; bk_state=%s]",
                bk->id_, bk->type_.c_str(), bk_state_to_string(STATE_START));

    bk->state_ = STATE_START;
    bk->start_();

    return true;
}

//
// @brief Stop a block
//
bool manager::block_stop(int id)
{
    struct block *bk;

    bk = block_get(id);
    if (bk == nullptr)
    {
        LOGGER_WARNING("Cannot stop block: unknown block ID [bk_id=%d]", id);
        return false;
    }

    // Verify block state
    if (bk->state_ == STATE_STOP)
    {
        // Nothing to do
        return true;
    }

    LOGGER_INFO("Stop block [bk_id=%d ; bk_type=%s ; bk_state=%s]",
                id, bk->type_.c_str(), bk_state_to_string(STATE_STOP));

    bk->state_ = STATE_STOP;

    bk->stop_();

    return true;
}

//
// @brief Stop and delete a block
//
// @param id : Block ID
//
bool manager::block_del(int id)
{
    struct block *bk;

    bk = block_get(id);
    if (bk == nullptr)
    {
        LOGGER_WARNING("Cannot delete block: unknown block ID [bk_id=%d]", id);
        return false;
    }

    // Verify block state
    if (bk->state_ != STATE_STOP)
    {
        LOGGER_WARNING("Cannot delete block: block not stopped [bk_id=%d ; bk_state=%s]",
                       bk->id_, bk_state_to_string(bk->state_));
        return false;
    }

    LOGGER_INFO("Delete block [bk_id=%d ; bk_type=%s]", bk->id_, bk->type_.c_str());

    const auto &factory = bk_factory_.find(bk->type_);
    if (factory == bk_factory_.cend())
    {
        LOGGER_ERR("Failed to delete block: no factory found [type=%s]", bk->type_.c_str());
        return false;
    }

    factory->second->destructor(bk);
    bk_map_.erase(id);

    return true;
}

//
// @brief Configure a block
//
bool manager::block_conf(int id, char *conf)
{
    struct block *bk;

    bk = block_get(id);
    if (bk == nullptr)
    {
        LOGGER_WARNING("Cannot configure block: unknown block ID [bk_id=%d]", id);
        return false;
    }

    LOGGER_INFO("Configure block [bk_id=%d ; bk_type=%s ; conf=%s]", id, bk->type_.c_str(), conf);

    bk->conf_(conf);

    return true;
}

//
// @brief Bind a block
//
bool manager::block_bind(int id, int port, int bk_id)
{
    struct bind_info bind;

    // Find the block concerned by the command
    const auto &source = bk_map_.find(id);
    const auto &end = bk_map_.end();
    if (source == end)
    {
        LOGGER_WARNING("Cannot bind block: unknown source [bk_id=%d]", id);
        return false;
    }

    LOGGER_INFO("Bind block [bk_id=%d ; port=%d ; bk_id_dest=%d]", source->first, port, bk_id);

    source->second->bind_(port, bk_id);

    // Add a binding in the engine with a weak reference to the block
    // If the block does not exist, we just let it undefined
    const auto &dest = bk_map_.find(bk_id);
    if (dest != end)
    {
        bind.bk = dest->second;
    }
    bind.port = port;
    bind.bk_id = bk_id;
    source->second->binds_.push_back(bind);

    return true;
}

//
// @brief Get block information
//
struct block *manager::block_get(int id)
{
    const auto &it = bk_map_.find(id);
    if (it == bk_map_.cend())
    {
        return nullptr;
    }
    return it->second;
}

//
// @brief Clear all blocks
//
void manager::block_clear()
{
    while (bk_map_.empty() == false)
    {
        const auto &it = bk_map_.cbegin();
        block_stop(it->first);
        block_del(it->first);
    }
}

void manager::block_factory_register(const char *type, struct block_factory *factory)
{
    bk_factory_.insert({type, factory});
}

void manager::block_factory_unregister(const char *type)
{
    bk_factory_.erase(type);
}

//
// @brief Clear the block factory
//
void manager::block_factory_clear()
{
    bk_factory_.clear();
}
