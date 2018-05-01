//
// @brief Test file for the hello block
//

// C++ library headers
#include <cstdlib> // NULL

// Project headers
#include "block/pub_sub.hpp"
#include "c3qo/block.hpp"
#include "c3qo/manager.hpp"
#include "utils/logger.hpp"
#include "utils/socket.hpp"

// Gtest library
#include "gtest/gtest.h"

// Managers shall be linked
extern struct manager *m;

// Client and server shall be linked
extern struct bk_if pub_sub_if;

class tu_pub_sub : public testing::Test
{
    void SetUp();
    void TearDown();
};

void tu_pub_sub::SetUp()
{
    LOGGER_OPEN("tu_pub_sub");
    logger_set_level(LOGGER_LEVEL_DEBUG);

    // Populate the managers
    m = new struct manager;
}

void tu_pub_sub::TearDown()
{
    // Clear the managers
    delete m;

    logger_set_level(LOGGER_LEVEL_NONE);
    LOGGER_CLOSE();
}

//
// @brief Nothing yet
//
TEST_F(tu_pub_sub, data)
{
    struct pub_sub_ctx *ctx_s; // server context
    struct pub_sub_ctx *ctx_c; // client context
    char conf_c[] = "type=client addr=tcp://127.0.0.1:5555";
    char conf_s[] = "type=server addr=tcp://127.0.0.1:5555";

    // Initialize two ZMQ publisher/subscriber
    ctx_s = (struct pub_sub_ctx *)pub_sub_if.init(1);
    ctx_c = (struct pub_sub_ctx *)pub_sub_if.init(2);
    ASSERT_NE(ctx_s, (void *)NULL);
    ASSERT_NE(ctx_c, (void *)NULL);

    // Configure them
    pub_sub_if.conf(ctx_c, conf_c);
    pub_sub_if.conf(ctx_s, conf_s);

    // Start them
    pub_sub_if.start(ctx_s);
    pub_sub_if.start(ctx_c);

    EXPECT_EQ(ctx_s->rx_pkt_count, 0lu);
    EXPECT_EQ(ctx_c->rx_pkt_count, 0lu);

    // Try to send some data between publisher and subscriber
    // Some messages are lost because subscription can take some time
    for (int i = 0; i < 10; i++)
    {
        struct c3qo_zmq_msg data;

        data.topic = (char *)"hello";
        data.topic_len = strlen(data.topic);
        data.data = (char *)"world";
        data.data_len = strlen(data.data);

        pub_sub_if.tx(ctx_c, (void *)&data);
        pub_sub_if.tx(ctx_s, (void *)&data);

        m->fd.poll_fd();
    }

    // At least one message should be received
    EXPECT_GT(ctx_c->rx_pkt_count, 1lu);
    EXPECT_GT(ctx_s->rx_pkt_count, 1lu);

    pub_sub_if.stop(ctx_s);
    pub_sub_if.stop(ctx_c);
}
