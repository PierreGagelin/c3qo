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

TEST_F(tu_project_euler, problem_1)
{
    char conf10[] = "1 10";
    char conf100[] = "1 100";
    char conf1000[] = "1 1000";

    project_euler_if.conf(NULL, conf10);
    project_euler_if.conf(NULL, conf100);
    project_euler_if.conf(NULL, conf1000);
}

TEST_F(tu_project_euler, problem_2)
{
    char conf10[] = "2 10";
    char conf100[] = "2 100";
    char conf1000[] = "2 1000";
    char conf4000000[] = "2 4000000";

    project_euler_if.conf(NULL, conf10);
    project_euler_if.conf(NULL, conf100);
    project_euler_if.conf(NULL, conf1000);
    project_euler_if.conf(NULL, conf4000000);
}


TEST_F(tu_project_euler, problem_3)
{
    char nada[] = "3 600851475143";

    project_euler_if.conf(NULL, nada);      //6857
}
