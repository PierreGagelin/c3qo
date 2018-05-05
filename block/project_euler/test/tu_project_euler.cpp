//
// @brief Test file for the project_euler block
//

// C++ library headers
#include <cstdlib> // NULL

// Project headers
#include "c3qo/block.hpp"
#include "c3qo/manager.hpp"
#include "utils/logger.hpp"

// Gtest library
#include "gtest/gtest.h"

// Managers shall be linked
extern struct manager *m;

// TU should be linked with the block
extern struct bk_if project_euler_if;

class tu_project_euler : public testing::Test
{
    void SetUp();
    void TearDown();
};

void tu_project_euler::SetUp()
{
    LOGGER_OPEN("tu_project_euler");
    logger_set_level(LOGGER_LEVEL_DEBUG);

    // Populate the managers
    m = new struct manager;
}

void tu_project_euler::TearDown()
{
    // Clear the managers
    delete m;

    logger_set_level(LOGGER_LEVEL_NONE);
    LOGGER_CLOSE();
}

//
// @brief Basic usage of block project_euler
//
TEST_F(tu_project_euler, problem_1)
{
    char conf10[] = "1 10";     // Solve problem 1 with argument 10
    char conf100[] = "1 100";   // Solve problem 1 with argument 100
    char conf1000[] = "1 1000"; // Solve problem 1 with argument 1000

    project_euler_if.conf(NULL, conf10);
    project_euler_if.conf(NULL, conf100);
    project_euler_if.conf(NULL, conf1000);
}
