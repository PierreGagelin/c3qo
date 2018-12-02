//
// @brief Test file for a block
//

// Project headers
#include "c3qo/manager.hpp"

// Gtest library
#include "gtest/gtest.h"

// Managers shall be linked
extern struct manager *m;

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
    struct bk_zmq_pair client(&mgr_);
    struct bk_zmq_pair server(&mgr_);
    char conf_s[] = "type=server addr=tcp://127.0.0.1:5555";
    char conf_c[] = "type=client addr=tcp://127.0.0.1:5555";

    // Initialize two ZMQ pairs
    server.id_ = 1;
    server.init_();
    client.id_ = 2;
    client.init_();
    ASSERT_NE(static_cast<struct zmq_pair_ctx *>(server.ctx_), nullptr);
    ASSERT_NE(static_cast<struct zmq_pair_ctx *>(client.ctx_), nullptr);

    // Configure them
    server.conf_(conf_s);
    client.conf_(conf_c);

    // Start them
    server.start_();
    client.start_();

    EXPECT_EQ(static_cast<struct zmq_pair_ctx *>(server.ctx_)->rx_pkt_count, 0lu);
    EXPECT_EQ(static_cast<struct zmq_pair_ctx *>(client.ctx_)->rx_pkt_count, 0lu);

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
    EXPECT_GT(static_cast<struct zmq_pair_ctx *>(client.ctx_)->rx_pkt_count, 1lu);
    EXPECT_GT(static_cast<struct zmq_pair_ctx *>(server.ctx_)->rx_pkt_count, 1lu);

    client.stop_();
    server.stop_();
}
