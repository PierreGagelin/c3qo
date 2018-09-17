//
// @brief Test file for a block
//

#define LOGGER_TAG "[TU.zmq_pair]"

// Project headers
#include "block/zmq_pair.hpp"
#include "c3qo/manager.hpp"

// Gtest library
#include "gtest/gtest.h"

class tu_zmq_pair : public testing::Test
{
    void SetUp();
    void TearDown();

  public:
    struct manager mgr_;
};

void tu_zmq_pair::SetUp()
{
    LOGGER_OPEN("tu_zmq_pair");
    logger_set_level(LOGGER_LEVEL_DEBUG);
}

void tu_zmq_pair::TearDown()
{
    logger_set_level(LOGGER_LEVEL_NONE);
    LOGGER_CLOSE();
}

//
// @brief Nothing yet
//
TEST_F(tu_zmq_pair, data)
{
    struct zmq_pair client(&mgr_);
    struct zmq_pair server(&mgr_);
    char conf_s[] = "type=server addr=tcp://127.0.0.1:5555";
    char conf_c[] = "type=client addr=tcp://127.0.0.1:5555";

    // Initialize two ZMQ pairs
    server.id_ = 1;
    client.id_ = 2;

    // Configure them
    server.conf_(conf_s);
    client.conf_(conf_c);

    // Start them
    server.start_();
    client.start_();

    EXPECT_EQ(server.rx_pkt_, 0lu);
    EXPECT_EQ(client.rx_pkt_, 0lu);

    // Send some data between both pairs
    // Some messages are lost because subscription can take some time
    for (int i = 0; i < 10; i++)
    {
        struct c3qo_zmq_msg msg;
        char topic[] = "hello";
        char data[] = "world";

        //FIXME: we shouldn't use such cast -> change API to use a std::string?
        msg.topic = topic;
        msg.topic_len = strlen(msg.topic);
        msg.data = data;
        msg.data_len = strlen(msg.data);

        client.tx_(&msg);
        server.tx_(&msg);

        mgr_.fd_poll();
    }

    // At least one message should be received
    EXPECT_GT(client.rx_pkt_, 0lu);
    EXPECT_GT(server.rx_pkt_, 0lu);

    client.stop_();
    server.stop_();
}
