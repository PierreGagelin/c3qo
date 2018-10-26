//
// @brief Test file for the block manager
//

#define LOGGER_TAG "[TU.c3qo.perf]"

// Project headers
#include "c3qo/tu.hpp"

class tu_perf : public testing::Test
{
    void SetUp();
    void TearDown();

  public:
    struct manager mgr_;
};

void tu_perf::SetUp()
{
    LOGGER_OPEN("tu_perf");
    logger_set_level(LOGGER_LEVEL_DEBUG);
}

void tu_perf::TearDown()
{
    logger_set_level(LOGGER_LEVEL_NONE);
    LOGGER_CLOSE();
}

//
// @brief Test the speed of commutation
//
TEST_F(tu_perf, commutation)
{
    size_t nb_block = 1 * 100;
    size_t nb_buf = 1 * 10 * 1000;

    // Reduce amount of output
    logger_set_level(LOGGER_LEVEL_WARNING);

    // Add, init and start some blocks
    for (size_t i = 1; i < nb_block + 1; i++)
    {
        EXPECT_EQ(mgr_.block_add(i, "hello"), true);
        EXPECT_EQ(mgr_.block_init(i), true);
        EXPECT_EQ(mgr_.block_start(i), true);
    }

    // Configure a chain of N blocks:
    //   - bk_1 -> bk_2 -> bk_3 -> bk_4... -> bk_N
    for (size_t i = 1; i < nb_block; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            // Bind port j of bk_i to bk_i+1
            EXPECT_EQ(mgr_.block_bind(i, j, i + 1), true);
        }
    }

    // Bind the last block to 0 (trash)
    for (int j = 0; j < 8; j++)
    {
        EXPECT_EQ(mgr_.block_bind(nb_block, j, 0), true);
    }

    // Send data from bk_1
    for (size_t i = 0; i < nb_buf; i++)
    {
        struct block *bi;
        char buf[] = "yolooooo";

        bi = mgr_.block_get(1);
        ASSERT_NE(bi, nullptr);

        bi->ctrl_(buf);
    }

    // Verify that buffers crossed bk_2 to the last block
    for (size_t i = 2; i < nb_block + 1; i++)
    {
        struct block *bi;
        char buf[16];
        size_t count;

        bi = mgr_.block_get(i);
        ASSERT_NE(bi, nullptr);

        bi->get_stats_(buf, sizeof(buf));
        count = static_cast<size_t>(std::stoul(buf));
        EXPECT_EQ(count, nb_buf);
    }

    // Clean blocks
    mgr_.block_clear();
}
