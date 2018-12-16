

#define LOGGER_TAG "[engine.block]"

// Project headers
#include "engine/manager.hpp"

//
// @brief Constructor and destructor
//
manager::manager() {}
manager::~manager()
{
    block_clear();
    block_factory_clear();
}

//
// @brief Add or replace a block
//
// @param id    : Identifier to give to the block
// @param type  : Type of block to create
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

    LOGGER_INFO("Added block [bk_id=%d ; bk_type=%s]", id, type);

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
        LOGGER_ERR("Failed to start block: unknown block [bk_id=%d]", id);
        return false;
    }

    // Verify block state
    if (bk->state_ == STATE_START)
    {
        // Nothing to do
        return true;
    }

    bk->state_ = STATE_START;
    bk->start_();

    LOGGER_INFO("Started block [bk_id=%d ; bk_type=%s]", bk->id_, bk->type_.c_str());

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
        LOGGER_ERR("Failed to stop block: unknown block [bk_id=%d]", id);
        return false;
    }

    // Verify block state
    if (bk->state_ == STATE_STOP)
    {
        // Nothing to do
        return true;
    }

    bk->state_ = STATE_STOP;
    bk->stop_();

    LOGGER_INFO("Stopped block [bk_id=%d ; bk_type=%s]", id, bk->type_.c_str());

    return true;
}

//
// @brief Stop and delete a block
//
bool manager::block_del(int id)
{
    struct block *bk;

    bk = block_get(id);
    if (bk == nullptr)
    {
        LOGGER_ERR("Failed to delete block: unknown block [bk_id=%d]", id);
        return false;
    }

    // Verify block state
    if (bk->state_ != STATE_STOP)
    {
        LOGGER_ERR("Failed to delete block: block not stopped [bk_id=%d]", bk->id_);
        return false;
    }

    const auto &factory = bk_factory_.find(bk->type_);
    if (factory == bk_factory_.cend())
    {
        LOGGER_ERR("Failed to delete block: unknown factory [type=%s]", bk->type_.c_str());
        return false;
    }

    LOGGER_INFO("Deleting block [bk_id=%d ; bk_type=%s]", bk->id_, bk->type_.c_str());

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
        LOGGER_ERR("Failed to configure block: unknown block [bk_id=%d]", id);
        return false;
    }
    bk->conf_(conf);

    LOGGER_INFO("Configured block [bk_id=%d ; bk_type=%s]", id, bk->type_.c_str());

    return true;
}

//
// @brief Bind a block
//
bool manager::block_bind(int bk_id_src, int port, int bk_id_dst)
{
    struct bind_info bind;

    // Find the block concerned by the command
    const auto &src = bk_map_.find(bk_id_src);
    const auto &dst = bk_map_.find(bk_id_dst);
    const auto &end = bk_map_.cend();
    if ((src == end) || (dst == end))
    {
        LOGGER_ERR("Failed to bind block: unknown block [bk_id_src=%d ; bk_id_dst=%d]", bk_id_src, bk_id_dst);
        return false;
    }

    bind.bk = dst->second;
    bind.port = port;
    src->second->binds_.push_back(bind);

    LOGGER_INFO("Bound block [bk_id_src=%d ; port=%d ; bk_id_dest=%d]", bk_id_src, port, bk_id_dst);

    // Notify the block that it has been bound
    src->second->bind_(port, bk_id_dst);

    return true;
}

//
// @brief Get a block
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

//
// @brief Register a type of block in the factory
//
void manager::block_factory_register(const char *type, struct block_factory *factory)
{
    bk_factory_.insert({type, factory});
}

//
// @brief Unregister a type of block in the factory
//
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
