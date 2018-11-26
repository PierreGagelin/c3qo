//
// @brief Test file for the block manager
//

#define LOGGER_TAG "[TU.c3qo.perf]"

// Project headers
#include "block/hello.hpp"
#include "c3qo/tu.hpp"

struct manager mgr_;

//
// @brief Test the speed of commutation
//
static void tu_perf_commutation()
{
    size_t nb_block = 1 * 100;
    size_t nb_buf = 1 * 10 * 1000;

    // Reduce amount of output
    logger_set_level(LOGGER_LEVEL_WARNING);

    // Add, init and start some blocks
    for (size_t i = 1; i < nb_block + 1; i++)
    {
        ASSERT(mgr_.block_add(i, "hello") == true);
        ASSERT(mgr_.block_start(i) == true);
    }

    // Configure a chain of N blocks:
    //   - bk_1 -> bk_2 -> bk_3 -> bk_4... -> bk_N
    for (size_t i = 1; i < nb_block; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            // Bind port j of bk_i to bk_i+1
            ASSERT(mgr_.block_bind(i, j, i + 1) == true);
        }
    }

    // Bind the last block to 0 (trash)
    for (int j = 0; j < 8; j++)
    {
        ASSERT(mgr_.block_bind(nb_block, j, 0) == true);
    }

    // Send data from bk_1
    for (size_t i = 0; i < nb_buf; i++)
    {
        struct block *bi;
        char buf[] = "yolooooo";

        bi = mgr_.block_get(1);
        ASSERT(bi != nullptr);

        bi->ctrl_(buf);
    }

    // Verify that buffers crossed bk_2 to the last block
    for (size_t i = 2; i < nb_block + 1; i++)
    {
        struct hello *bi;

        bi = static_cast<struct hello *>(mgr_.block_get(i));
        ASSERT(bi != nullptr);
        ASSERT(bi->count_ == static_cast<int>(nb_buf));
    }

    // Clean blocks
    mgr_.block_clear();
}

int main(int, char **)
{
    LOGGER_OPEN("tu_perf");
    logger_set_level(LOGGER_LEVEL_DEBUG);

    tu_perf_commutation();

    LOGGER_CLOSE();
    return 0;
}
