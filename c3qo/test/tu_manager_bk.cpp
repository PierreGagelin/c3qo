//
// @brief Test file for the block manager
//

// c3qo test unit library
#include "c3qo/tu.hpp"

// Generic purpose block structure
struct block_derived : block
{
    explicit block_derived(struct manager *mgr) : block(mgr) {}
    ~block_derived() {}
};

BLOCK_REGISTER(block_derived);

class tu_manager_bk : public testing::Test
{
    void SetUp()
    {
        LOGGER_OPEN("tu_manager_bk");
        logger_set_level(LOGGER_LEVEL_DEBUG);
    }
    void TearDown()
    {
        logger_set_level(LOGGER_LEVEL_NONE);
        LOGGER_CLOSE();
    }

  public:
    struct manager mgr_;
};

//
// @brief Test creation and use of default block
//
TEST_F(tu_manager_bk, block)
{
    struct block_derived bk(&mgr_);

    bk.init_();
    bk.conf_(nullptr);
    bk.bind_(0, 0);
    bk.start_();
    bk.stop_();
    EXPECT_EQ(bk.get_stats_(nullptr, 0u), 0u);
    EXPECT_EQ(bk.rx_(nullptr), 0);
    EXPECT_EQ(bk.tx_(nullptr), 0);
    EXPECT_EQ(bk.ctrl_(nullptr), 0);
}

//
// @brief Test the data flow between blocks
//
// For this test, we need to use the statically defined manager of block
//
TEST_F(tu_manager_bk, flow)
{
    struct block *bk_1;
    struct block *bk_2;
    char stats[] = "useless value";
    int count;

    // Add, initialize and start 2 blocks
    for (int i = 1; i < 3; i++)
    {
        EXPECT_EQ(mgr_.block_add(i, "hello"), true);
        EXPECT_EQ(mgr_.block_init(i), true);
        EXPECT_EQ(mgr_.block_start(i), true);
    }

    // Bind:
    //   - block 1 to block 2
    //   - block 2 to block 0 (trash)
    for (int i = 0; i < 8; i++)
    {
        EXPECT_EQ(mgr_.block_bind(1, i, 2), true);
        EXPECT_EQ(mgr_.block_bind(2, i, 0), true);
    }

    // Retrieve block 1 and block 2
    bk_1 = mgr_.block_get(1);
    bk_2 = mgr_.block_get(2);
    ASSERT_NE(bk_1, nullptr);
    ASSERT_NE(bk_2, nullptr);

    // No data should have gone through blocks
    bk_1->get_stats_(stats, sizeof(stats));
    count = atoi(stats);
    EXPECT_TRUE(count == 0);
    bk_2->get_stats_(stats, sizeof(stats));
    count = atoi(stats);
    EXPECT_TRUE(count == 0);

    // Notify the block to generate a TX data flow: it shall return 0
    EXPECT_TRUE(bk_1->ctrl_(stats) == 0);

    // A buffer should have crossed block 2
    bk_1->get_stats_(stats, sizeof(stats));
    count = atoi(stats);
    EXPECT_EQ(count, 0);
    bk_2->get_stats_(stats, sizeof(stats));
    count = atoi(stats);
    EXPECT_EQ(count, 1);

    // Clear blocks
    mgr_.block_clear();
}

//
// @brief String version of the block enumerates
//
TEST_F(tu_manager_bk, strings)
{
    for (int i = 0; i < 10; i++)
    {
        bk_cmd_to_string((enum bk_cmd)i);
        bk_state_to_string((enum bk_state)i);
    }
}
