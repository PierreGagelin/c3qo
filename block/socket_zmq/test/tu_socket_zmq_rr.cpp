//
// @brief Test file for the hello block
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

class tu_socket_zmq_rr : public testing::Test
{
    void SetUp();
    void TearDown();
};

void tu_socket_zmq_rr::SetUp()
{
    LOGGER_OPEN("tu_socket_zmq_rr");
    logger_set_level(LOGGER_LEVEL_DEBUG);

    // Populate the managers
    m = new struct manager;
}

void tu_socket_zmq_rr::TearDown()
{
    // Clear the managers
    delete m;
    
    logger_set_level(LOGGER_LEVEL_NONE);
    LOGGER_CLOSE();
}

//
// @brief Nothing yet
//
TEST_F(tu_socket_zmq_rr, nothing)
{
}
