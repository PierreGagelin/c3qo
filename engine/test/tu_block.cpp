//
// @brief Test file for the block interface
//

// Project headers
#include "block/hello.hpp"
#include "engine/tu.hpp"

// Generic purpose block structure
struct block_derived : block
{
    explicit block_derived(struct manager *mgr) : block(mgr) {}
    virtual ~block_derived() override final {}
};

struct hello_factory factory;
struct manager mgr_;

//
// @brief Test creation and use of default block
//
static void tu_block_interface()
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
static void tu_block_flow()
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

    // Generate flow from the block
    bk_1->process_notif_(0, notif);
    bk_1->process_rx_(0, notif);
    bk_1->process_tx_(0, notif);

    // A buffer should have crossed block 2
    ASSERT(bk_1->count_ == 0);
    ASSERT(bk_2->count_ == 3);

    // Process unknown flow
    bk_1->process_flow_(0, nullptr, static_cast<enum flow_type>(42));

    // Clear blocks
    mgr_.block_clear();
}

//
// @brief String version of the block enumerates
//
static void tu_block_strings()
{
    for (int i = 0; i < 10; i++)
    {
        bk_cmd_to_string(static_cast<enum bk_cmd>(i));
        bk_state_to_string(static_cast<enum bk_state>(i));
        flow_type_to_string(static_cast<enum flow_type>(i));
    }
}

int main(int, char **)
{
    LOGGER_OPEN("tu_block");
    logger_set_level(LOGGER_LEVEL_DEBUG);

    mgr_.block_factory_register("hello", &factory);

    tu_block_interface();
    tu_block_flow();
    tu_block_strings();

    LOGGER_CLOSE();
    return 0;
}
