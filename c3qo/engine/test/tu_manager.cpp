/**
 * @brief Test file for the block manager
 */


//#include <iostream> 
#include <fstream>  // open, close

extern "C" 
{
#include "c3qo/logger.h"  // LOGGER_OPEN, LOGGER_CLOSE
#include "c3qo/manager.h" // manager_parse_conf
}

#include "gtest/gtest.h"


class tu_manager : public testing::Test
{
        void SetUp();
        void TearDown();
};

void tu_manager::SetUp()
{
        LOGGER_OPEN();
}

void tu_manager::TearDown()
{
        LOGGER_CLOSE();
}


TEST_F(tu_manager, manager)
{
        const char * filename = "/tmp/tu_manager_config.txt";
        std::fstream file;

        file.open(filename, std::ios::out | std::ios::trunc);
        ASSERT_EQ(file.is_open(), true);

        file << "1 0 1\n";

        file.close();

        EXPECT_EQ(manager_parse_conf(filename), true);
}


