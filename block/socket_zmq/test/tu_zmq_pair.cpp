//
// @brief Test file for a block
//

// Project headers
#include "c3qo/manager.hpp"

// Gtest library
#include "gtest/gtest.h"

// Managers shall be linked
extern struct manager *m;

// Client and server shall be linked
extern struct bk_if zmq_pair_if;

class tu_zmq_pair : public testing::Test
{
    void SetUp();
    void TearDown();
};

void tu_zmq_pair::SetUp()
{
    LOGGER_OPEN("tu_zmq_pair");
    logger_set_level(LOGGER_LEVEL_DEBUG);

    // Populate the managers
    m = new struct manager;
}

void tu_zmq_pair::TearDown()
{
    // Clear the managers
    delete m;

    logger_set_level(LOGGER_LEVEL_NONE);
    LOGGER_CLOSE();
}

//
// @brief Nothing yet
//
TEST_F(tu_zmq_pair, data)
{
    struct zmq_pair_ctx *ctx_s; // server context
    struct zmq_pair_ctx *ctx_c; // client context
    char conf_s[] = "type=server addr=tcp://127.0.0.1:5555";
    char conf_c[] = "type=client addr=tcp://127.0.0.1:5555";

    // Initialize two ZMQ pairs
    ctx_s = (struct zmq_pair_ctx *)zmq_pair_if.init(1);
    ctx_c = (struct zmq_pair_ctx *)zmq_pair_if.init(2);
    ASSERT_NE(ctx_s, nullptr);
    ASSERT_NE(ctx_c, nullptr);

    // Configure them
    zmq_pair_if.conf(ctx_s, conf_s);
    zmq_pair_if.conf(ctx_c, conf_c);

    // Start them
    zmq_pair_if.start(ctx_s);
    zmq_pair_if.start(ctx_c);

    EXPECT_EQ(ctx_s->rx_pkt_count, 0lu);
    EXPECT_EQ(ctx_c->rx_pkt_count, 0lu);

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

        zmq_pair_if.tx(ctx_s, (void *)&msg);
        zmq_pair_if.tx(ctx_c, (void *)&msg);

        m->fd.poll_fd();
    }

    // At least one message should be received
    EXPECT_GT(ctx_c->rx_pkt_count, 1lu);
    EXPECT_GT(ctx_s->rx_pkt_count, 1lu);

    zmq_pair_if.stop(ctx_s);
    zmq_pair_if.stop(ctx_c);
}
