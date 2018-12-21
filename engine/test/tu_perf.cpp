//
// @brief Test file for the block manager
//

// Project headers
#include "block/hello.hpp"
#include "engine/tu.hpp"

struct hello_factory factory;
struct manager mgr_;

//
// @brief Test the speed of commutation
//
static void tu_perf_commutation()
{
    size_t nb_block = 1 * 100;
    size_t nb_buf = 1 * 10 * 1000;
    struct block *bk;

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
        for (int j = 1; j < 9; j++)
        {
            // Bind port j of bk_i to bk_i+1
            ASSERT(mgr_.block_bind(i, j, i + 1) == true);
        }
    }

    // Send data from bk_1
    bk = mgr_.block_get(1);
    ASSERT(bk != nullptr);
    for (size_t i = 0; i < nb_buf; i++)
    {
        bk->process_data_(1, nullptr);
    }

    // Verify that buffers crossed bk_2 to the last block
    for (size_t i = 2; i < nb_block + 1; i++)
    {
        struct hello *bk_hello;

        bk_hello = static_cast<struct hello *>(mgr_.block_get(i));
        ASSERT(bk_hello != nullptr);
        ASSERT(bk_hello->count_ == static_cast<int>(nb_buf));
    }

    // Clean blocks
    mgr_.block_clear();
}

int main(int, char **)
{
    LOGGER_OPEN("tu_perf");
    logger_set_level(LOGGER_LEVEL_CRIT);

    mgr_.block_factory_register("hello", &factory);

    tu_perf_commutation();

    LOGGER_CLOSE();
    return 0;
}
