//
// @brief Test file for the block interface
//

// Project headers
#include "block/hello.hpp"
#include "engine/tu.hpp"

struct hello_factory factory;
struct manager mgr_;

//
// @brief Test creation and use of default block
//
// Dynamic allocation is required to hit every destructors
//
static void tu_block_interface()
{
    struct block *bk = new struct block(&mgr_);

    bk->bind_(0, 0);
    bk->start_();
    bk->stop_();

    ASSERT(bk->data_(nullptr) == 0);
    bk->ctrl_(nullptr);

    struct timer t;
    bk->on_timer_(t);

    struct file_desc f;
    bk->on_fd_(f);

    delete bk;
}

//
// @brief Test the data flow between blocks
//
static void tu_block_flow()
{
    struct hello *bk_1;
    struct hello *bk_2;

    // Add, initialize and start 2 blocks
    for (int i = 1; i < 3; ++i)
    {
        ASSERT(mgr_.block_add(i, "hello") == true);
        ASSERT(mgr_.block_start(i) == true);
    }

    // Bind block 1 to block 2
    for (int i = 1; i < 9; ++i)
    {
        ASSERT(mgr_.block_bind(1, i, 2) == true);
    }

    // Retrieve block 1 and block 2
    bk_1 = static_cast<struct hello *>(mgr_.block_get(1));
    bk_2 = static_cast<struct hello *>(mgr_.block_get(2));
    ASSERT(bk_1 != nullptr);
    ASSERT(bk_2 != nullptr);

    // No data should have gone through blocks
    ASSERT(bk_1->count_ == 0);
    ASSERT(bk_2->count_ == 0);

    // Control a block
    bk_1->process_ctrl_(1, nullptr);
    ASSERT(bk_1->count_ == 1);
    bk_2->process_ctrl_(2, nullptr);
    ASSERT(bk_2->count_ == 1);

    // Generate flow from the block
    bk_1->process_data_(1, nullptr);
    ASSERT(bk_2->count_ == 2);
    bk_1->process_data_(1, nullptr);
    ASSERT(bk_2->count_ == 3);

    // Clear blocks
    mgr_.block_clear();
}

static void tu_block_errors()
{
    struct hello *bk;

    ASSERT(mgr_.block_add(1, "hello") == true);
    ASSERT(mgr_.block_add(2, "hello") == true);

    bk = static_cast<struct hello *>(mgr_.block_get(1));
    ASSERT(bk != nullptr);

    // Control unknown block
    bk->process_ctrl_(42, nullptr);

    // Flow without a route
    bk->process_data_(1, nullptr);
}

int main(int, char **)
{
    LOGGER_OPEN("tu_block");
    logger_set_level(LOGGER_LEVEL_DEBUG);

    mgr_.block_factory_register("hello", &factory);

    tu_block_interface();
    tu_block_flow();
    tu_block_errors();

    LOGGER_CLOSE();
    return 0;
}
