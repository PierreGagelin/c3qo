//
// @brief Test file for the hello block
//

// C++ library headers

// Project headers
#include "utils/logger.hpp"

// Gtest library
#include "gtest/gtest.h"

#include "test.pb.h"

class tu_pb_person : public testing::Test
{
    void SetUp();
    void TearDown();
};

void tu_pb_person::SetUp()
{
    LOGGER_OPEN("tu_pb_person");
    logger_set_level(LOGGER_LEVEL_DEBUG);
}

void tu_pb_person::TearDown()
{
    google::protobuf::ShutdownProtobufLibrary();

    logger_set_level(LOGGER_LEVEL_NONE);
    LOGGER_CLOSE();
}

//
// @brief Basic usage of block hello
//
TEST_F(tu_pb_person, hello)
{
    class person p;
}
