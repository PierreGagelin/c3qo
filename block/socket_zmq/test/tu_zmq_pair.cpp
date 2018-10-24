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
    void SetUp()
    {
        LOGGER_OPEN("tu_zmq_pair");
        logger_set_level(LOGGER_LEVEL_DEBUG);
    }
    void TearDown()
    {
        logger_set_level(LOGGER_LEVEL_NONE);
        LOGGER_CLOSE();
    }

  public:
    struct manager mgr_;
};

void message_create(std::vector<struct c3qo_zmq_part> &msg, const char *topic, const char *payload)
{
    struct c3qo_zmq_part part;

    part.data = strdup(topic);
    ASSERT(part.data != nullptr);
    part.len = strlen(part.data);
    msg.push_back(part);

    part.data = strdup(payload);
    ASSERT(part.data != nullptr);
    part.len = strlen(part.data);
    msg.push_back(part);
}

void message_destroy(std::vector<struct c3qo_zmq_part> &msg)
{
    for (const auto &it : msg)
    {
        free(it.data);
    }
    msg.clear();
}

//
// @brief Verify data transmission between client and server
//
TEST_F(tu_zmq_pair, data)
{
    struct zmq_pair client(&mgr_);
    struct zmq_pair server(&mgr_);
    char conf_s[] = "type=server addr=tcp://127.0.0.1:5555";
    char conf_c[] = "type=client addr=tcp://127.0.0.1:5555";
    std::vector<struct c3qo_zmq_part> msg;

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
        message_create(msg, "hello", "world");

        client.tx_(&msg);
        server.tx_(&msg);

        mgr_.fd_poll();

        message_destroy(msg);
    }

    // At least one message should be received
    EXPECT_GT(client.rx_pkt_, 0lu);
    EXPECT_GT(server.rx_pkt_, 0lu);

    // Send several of the expected messages
    {
        message_create(msg, "STATS", "HELLO");
        client.tx_(&msg);
        mgr_.fd_poll();
        message_destroy(msg);

        message_create(msg, "CONF.LINE", "1 2 3");
        client.tx_(&msg);
        mgr_.fd_poll();
        message_destroy(msg);
    }


    client.stop_();
    server.stop_();
}

// Test error cases
TEST_F(tu_zmq_pair, error)
{
    struct zmq_pair block(&mgr_);

    block.id_ = 1;

    block.conf_(nullptr);

    // Type error
    block.conf_(const_cast<char *>("hello"));
    block.conf_(const_cast<char *>("type="));
    block.conf_(const_cast<char *>("type= "));
    block.conf_(const_cast<char *>("type=banana "));

    // Address error
    block.conf_(const_cast<char *>("type=client addr="));
    block.conf_(const_cast<char *>("type=client addr= "));

    block.tx_(nullptr);
}
