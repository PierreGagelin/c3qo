//
// @brief Test file for the hello block
//

// System C headers

// C++ library headers

// Project headers
#include "utils/logger.hpp"

// Gtest library
#include "gtest/gtest.h"

// Protobuf library
#include <google/protobuf/message_lite.h>                  // SerializeToZeroCopyStream, ParseFromZeroCopyStream
#include <google/protobuf/io/zero_copy_stream.h>           // ZeroCopyInputStream, ZeroCopyOutputStream
#include <google/protobuf/io/zero_copy_stream_impl_lite.h> // StringOutputStream, ArrayInputStream

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
// @brief Basic usage of protobuf message and serialization
//
TEST_F(tu_pb_person, string_serialize_parse)
{
    class person p_in;
    class person p_out;
    google::protobuf::io::ZeroCopyInputStream *input;
    google::protobuf::io::ZeroCopyOutputStream *output;
    std::string s_out;

    p_in.set_id1(1);
    p_in.set_id2(2);
    p_in.set_id3(3);
    p_in.set_id4(4);
    p_in.set_id5(5);
    EXPECT_EQ(p_in.id1(), 1);
    EXPECT_EQ(p_in.id2(), 2);
    EXPECT_EQ(p_in.id3(), 3);
    EXPECT_EQ(p_in.id4(), 4);
    EXPECT_EQ(p_in.id5(), 5);

    p_in.set_toto("Toto de l'ASTICOT");
    EXPECT_EQ(p_in.toto(), "Toto de l'ASTICOT");

    // Serialize the message into the output file
    output = new google::protobuf::io::StringOutputStream(&s_out);
    EXPECT_TRUE(p_in.SerializeToZeroCopyStream(output));

    // Parse the message from the input file
    input = new google::protobuf::io::ArrayInputStream(s_out.c_str(), s_out.length());
    EXPECT_TRUE(p_out.ParseFromZeroCopyStream(input));

    EXPECT_EQ(p_out.id1(), 1);
    EXPECT_EQ(p_out.id2(), 2);
    EXPECT_EQ(p_out.id3(), 3);
    EXPECT_EQ(p_out.id4(), 4);
    EXPECT_EQ(p_out.id5(), 5);
    EXPECT_EQ(p_out.toto(), std::string("Toto de l'ASTICOT"));

    delete input;
    delete output;
}
