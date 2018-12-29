//
// @brief Test file for the block manager
//

// Project headers
#include "engine/tu.hpp"

// Generic purpose block structure
struct block_derived : block
{
    explicit block_derived(struct manager *mgr) : block(mgr) {}
    virtual ~block_derived() override final {}
};

// Wrong factory
struct factory_failure : block_factory
{
    virtual struct block *constructor(struct manager *) override final { return nullptr; }
    virtual void destructor(struct block *) override final {}
};

// Correct factory
struct factory_success : block_factory
{
    virtual struct block *constructor(struct manager *mgr) override final
    {
        return new struct block_derived(mgr);
    }
    virtual void destructor(struct block *bk) override final
    {
        delete static_cast<struct block_derived *>(bk);
    }
};

struct manager mgr_;

//
// @brief Test block cycle of life
//
static void tu_manager_bk_life_cycle()
{
    struct factory_failure failure;
    struct factory_success success;

    // Can't add block 0
    ASSERT(mgr_.block_add(0, "block_derived") == false);

    // Can't add block with unregistered factory
    ASSERT(mgr_.block_add(1, "dummy") == false);

    // Can't add a block if allocation fails
    mgr_.block_factory_register("block_derived", &failure);
    ASSERT(mgr_.block_add(1, "block_derived") == false);
    mgr_.block_factory_unregister("block_derived");

    // Can add if the factory works
    mgr_.block_factory_register("block_derived", &success);
    ASSERT(mgr_.block_add(1, "block_derived") == true);

    // Can't add the same ID several times
    ASSERT(mgr_.block_add(-1, "block_derived") == true);
    ASSERT(mgr_.block_add(-1, "block_derived") == false);

    // Can't start unknown block
    ASSERT(mgr_.block_start(2) == false);

    // Can start several times the same block
    // will actually only be started once
    ASSERT(mgr_.block_start(1) == true);
    ASSERT(mgr_.block_start(1) == true);

    // Can't delete a started block (need to stop it first)
    ASSERT(mgr_.block_del(1) == false);

    // Can't stop unknown block
    ASSERT(mgr_.block_stop(2) == false);

    // Can stop several times the same block
    // will actually only be stopped once
    ASSERT(mgr_.block_stop(1) == true);
    ASSERT(mgr_.block_stop(1) == true);

    // Can't delete a block without a factory
    mgr_.block_factory_unregister("block_derived");
    ASSERT(mgr_.block_del(1) == false);

    // Can delete the block with its factory
    mgr_.block_factory_register("block_derived", &success);
    ASSERT(mgr_.block_del(1) == true);

    mgr_.block_clear();
    mgr_.block_factory_clear();
}

int main(int, char **)
{
    LOGGER_OPEN("tu_manager_bk");

    tu_manager_bk_life_cycle();

    LOGGER_CLOSE();
    return 0;
}
