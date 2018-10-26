//
// @brief Test file for the project_euler block
//

#define LOGGER_TAG "[TU.project_euler]"

// Project headers
#include "block/project_euler.hpp"
#include "c3qo/tu.hpp"

class tu_project_euler : public testing::Test
{
    void SetUp();
    void TearDown();

  public:
    struct manager mgr_;
};

void tu_project_euler::SetUp()
{
    LOGGER_OPEN("tu_project_euler");
    logger_set_level(LOGGER_LEVEL_DEBUG);
}

void tu_project_euler::TearDown()
{
    logger_set_level(LOGGER_LEVEL_NONE);
    LOGGER_CLOSE();
}

TEST_F(tu_project_euler, problem_1)
{
    struct project_euler block(&mgr_);
    char conf10[] = "1 10";
    char conf100[] = "1 100";
    char conf1000[] = "1 1000";

    block.conf_(conf10);
    block.conf_(conf100);
    block.conf_(conf1000);
}

TEST_F(tu_project_euler, problem_2)
{
    struct project_euler block(&mgr_);
    char conf10[] = "2 10";
    char conf100[] = "2 100";
    char conf1000[] = "2 1000";

    block.conf_(conf10);
    block.conf_(conf100);
    block.conf_(conf1000);
}


TEST_F(tu_project_euler, problem_3)
{
    struct project_euler block(&mgr_);
    char conf[] = "3 600851475143";

    block.conf_(conf);
}


TEST_F(tu_project_euler, problem_51)
{
    struct project_euler block(&mgr_);
    char conf[] = "51 useless";

    block.conf_(conf);
}
