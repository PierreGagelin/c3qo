//
// @brief Test file for the block manager
//

// Project headers
#include "block/hello.hpp"
#include "c3qo/tu.hpp"

// Generic purpose block structure
struct block_derived : block
{
    explicit block_derived(struct manager *mgr) : block(mgr) {}
    virtual ~block_derived() override final {}
};

struct block *beautiful_ptr;

// Add a dynamic constructor that "fails"
extern "C"
{
    struct block *derived_block_create(struct manager *)
    {
        return beautiful_ptr;
    }
}

struct manager mgr_;

//
// @brief Test creation and use of default block
//
static void tu_manager_bk_interface()
{
    struct block_derived bk(&mgr_);

    bk.conf_(nullptr);
    bk.bind_(0, 0);
    bk.start_();
    bk.stop_();

    ASSERT(bk.rx_(nullptr) == 0);
    ASSERT(bk.tx_(nullptr) == 0);
    ASSERT(bk.ctrl_(nullptr) == 0);

    struct timer t;
    bk.on_timer_(t);

    struct file_desc f;
    bk.on_fd_(f);
}

//
// @brief Test the data flow between blocks
//
// For this test, we need to use the statically defined manager of block
//
static void tu_manager_bk_flow()
{
    struct hello *bk_1;
    struct hello *bk_2;
    char notif[] = "dummy value";

    // Add, initialize and start 2 blocks
    for (int i = 1; i < 3; i++)
    {
        ASSERT(mgr_.block_add(i, "hello") == true);
        ASSERT(mgr_.block_start(i) == true);
    }

    // Bind:
    //   - block 1 to block 2
    //   - block 2 to block 0 (trash)
    for (int i = 0; i < 8; i++)
    {
        ASSERT(mgr_.block_bind(1, i, 2) == true);
        ASSERT(mgr_.block_bind(2, i, 0) == true);
    }

    // Retrieve block 1 and block 2
    bk_1 = static_cast<struct hello *>(mgr_.block_get(1));
    bk_2 = static_cast<struct hello *>(mgr_.block_get(2));
    ASSERT(bk_1 != nullptr);
    ASSERT(bk_2 != nullptr);

    // No data should have gone through blocks
    ASSERT(bk_1->count_ == 0);
    ASSERT(bk_2->count_ == 0);

    // Notify the block to generate a TX data flow: it shall return 0
    ASSERT(bk_1->ctrl_(notif) == 0);

    // A buffer should have crossed block 2
    ASSERT(bk_1->count_ == 0);
    ASSERT(bk_2->count_ == 1);

    // Clear blocks
    mgr_.block_clear();
}

//
// @brief String version of the block enumerates
//
static void tu_manager_bk_strings()
{
    for (int i = 0; i < 10; i++)
    {
        bk_cmd_to_string(static_cast<enum bk_cmd>(i));
        bk_state_to_string(static_cast<enum bk_state>(i));
        flow_type_to_string(static_cast<enum flow_type>(i));
    }
}

//
// @brief Test edge cases
//
static void tu_manager_bk_life_cycle()
{
    struct hello *bk = new struct hello(&mgr_);

    // Can't add block 0
    ASSERT(mgr_.block_add(0, "hello") == false);

    // Can't add block with unknown symbol name
    ASSERT(mgr_.block_add(1, "dummy") == false);

    // Can't add a block if allocation fails
    beautiful_ptr = nullptr;
    ASSERT(mgr_.block_add(-1, "derived_block") == false);

    // Can't delete a block if destructor isn't found
    beautiful_ptr = bk;
    ASSERT(mgr_.block_add(-1, "derived_block") == true);
    ASSERT(mgr_.block_del(-1) == false);

    // Delete the block that remains
    bk->type_ = "hello";
    ASSERT(mgr_.block_del(-1) == true);

    // Can't add the same ID several times
    ASSERT(mgr_.block_add(1, "hello") == true);
    ASSERT(mgr_.block_add(1, "hello") == false);

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

    // Can delete a stopped block
    ASSERT(mgr_.block_del(1) == true);
}

int main(int, char **)
{
    LOGGER_OPEN("tu_manager_bk");
    logger_set_level(LOGGER_LEVEL_DEBUG);

    tu_manager_bk_interface();
    tu_manager_bk_flow();
    tu_manager_bk_strings();
    tu_manager_bk_life_cycle();

    LOGGER_CLOSE();
    return 0;
}
