

#define LOGGER_TAG "[engine.block]"

// Project headers
#include "c3qo/manager.hpp"

// C headers
extern "C"
{
#include <dlfcn.h>
}

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
}

//
// @brief Add or replace a block
//
// @param id   : Block ID
// @param type : Block type
//
bool manager::block_add(int id, const char *type)
{
    struct block *(*constructor)(struct manager *);
    void *handle;
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

    handle = dlopen(nullptr, RTLD_LAZY);
    if (handle == nullptr)
    {
        LOGGER_ERR("Failed to add block: dlopen failed: %s [bk_id=%d ; bk_type=%s]", dlerror(), id, type);
        return false;
    }

    std::string ctor_name = std::string(type) + "_create";
    constructor = reinterpret_cast<struct block *(*)(struct manager *)>(dlsym(handle, ctor_name.c_str()));
    if (constructor == nullptr)
    {
        LOGGER_ERR("Failed to add block: couldn't load constructor [bk_id=%d ; bk_type=%s ; ctor_symbol=%s]",
                   id, type, ctor_name.c_str());
        goto err;
    }
    bk = constructor(this);
    if (bk == nullptr)
    {
        LOGGER_ERR("Failed to add block: construction failed [ctor_name=%s]", ctor_name.c_str());
        goto err;
    }

    bk->id_ = id;
    bk->type_ = type;
    bk->state_ = STATE_STOP;

    LOGGER_INFO("Add block [bk_id=%d ; bk_type=%s]", bk->id_, bk->type_.c_str());

    bk_map_.insert({id, bk});

    dlclose(handle);
    return true;
err:
    dlclose(handle);
    return false;
}

//
// @brief Stop a block
//
bool manager::block_start(int id)
{
    // Find the block concerned by the command
    const auto &it = bk_map_.find(id);
    if (it == bk_map_.end())
    {
        LOGGER_WARNING("Cannot start block: unknown block ID [bk_id=%d]", id);
        return false;
    }

    // Verify block state
    if (it->second->state_ == STATE_START)
    {
        // Nothing to do
        return true;
    }

    LOGGER_INFO("Start block [bk_id=%d ; bk_type=%s ; bk_state=%s]",
                it->second->id_, it->second->type_.c_str(), bk_state_to_string(STATE_START));

    it->second->state_ = STATE_START;

    it->second->start_();

    return true;
}

//
// @brief Stop a block
//
bool manager::block_stop(int id)
{
    // Find the block concerned by the command
    const auto &it = bk_map_.find(id);
    if (it == bk_map_.end())
    {
        LOGGER_WARNING("Cannot stop block: unknown block ID [bk_id=%d]", id);
        return false;
    }

    // Verify block state
    if (it->second->state_ == STATE_STOP)
    {
        // Nothing to do
        return true;
    }

    LOGGER_INFO("Stop block [bk_id=%d ; bk_type=%s ; bk_state=%s]",
                id, it->second->type_.c_str(), bk_state_to_string(STATE_STOP));

    it->second->state_ = STATE_STOP;

    it->second->stop_();

    return true;
}

//
// @brief Stop and delete a block
//
// @param id : Block ID
//
bool manager::block_del(int id)
{
    const auto &it = bk_map_.find(id);
    if (it == bk_map_.end())
    {
        LOGGER_WARNING("Cannot delete block: unknown block ID [bk_id=%d]", id);
        return false;
    }

    // Verify block state
    if (it->second->state_ != STATE_STOP)
    {
        LOGGER_WARNING("Cannot delete block: block not stopped [bk_id=%d ; bk_state=%s]",
                       id, bk_state_to_string(it->second->state_));
        return false;
    }

    LOGGER_INFO("Delete block [bk_id=%d ; bk_type=%s]", it->second->id_, it->second->type_.c_str());

    delete it->second;

    bk_map_.erase(it);

    return true;
}

//
// @brief Configure a block
//
bool manager::block_conf(int id, char *conf)
{
    // Find the block concerned by the command
    const auto &it = bk_map_.find(id);
    if (it == bk_map_.end())
    {
        LOGGER_WARNING("Cannot configure block: unknown block ID [bk_id=%d]", id);
        return false;
    }

    LOGGER_INFO("Configure block [bk_id=%d ; bk_type=%s ; conf=%s]", id, it->second->type_.c_str(), conf);

    it->second->conf_(conf);

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
