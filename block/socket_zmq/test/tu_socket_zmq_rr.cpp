//
// @brief Test file for the hello block
//

// C++ library headers
#include <cstdlib> // NULL

// Project headers
#include "block/server_zmq_rr.hpp"
#include "block/client_zmq_rr.hpp"
#include "c3qo/block.hpp"
#include "c3qo/manager.hpp"
#include "utils/logger.hpp"

// Gtest library
#include "gtest/gtest.h"

// Managers shall be linked
extern struct manager *m;

// Client and server shall be linked
extern struct bk_if client_zmq_rr_if;
extern struct bk_if server_zmq_rr_if;

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
TEST_F(tu_socket_zmq_rr, data)
{
    struct client_zmq_rr_ctx *ctx_s; // server context
    struct client_zmq_rr_ctx *ctx_c; // client context
    char conf_c[] = "pub_addr=tcp://*:6666 sub_addr=tcp://127.0.0.1:5555";
    char conf_s[] = "pub_addr=tcp://*:5555 sub_addr=tcp://127.0.0.1:6666";

    // Initialize two ZMQ publisher/subscriber
    ctx_s = (struct client_zmq_rr_ctx *)client_zmq_rr_if.init(1);
    ctx_c = (struct client_zmq_rr_ctx *)client_zmq_rr_if.init(2);
    ASSERT_NE(ctx_s, (void *)NULL);
    ASSERT_NE(ctx_c, (void *)NULL);

    // Configure them
    client_zmq_rr_if.conf(ctx_c, conf_c);
    client_zmq_rr_if.conf(ctx_s, conf_s);

    // Start them
    client_zmq_rr_if.start(ctx_s);
    client_zmq_rr_if.start(ctx_c);

    EXPECT_EQ(ctx_s->rx_pkt_count, 0lu);
    EXPECT_EQ(ctx_c->rx_pkt_count, 0lu);

    // Try to send some data between publisher and subscriber
    // Some messages are lost because subscription can take some time
    for (int i = 0; i < 10; i++)
    {
        client_zmq_rr_if.tx(ctx_c, (void *)"");
        client_zmq_rr_if.tx(ctx_s, (void *)"");
        m->fd.poll_fd();
    }

    // At least one message should be received
    EXPECT_GT(ctx_c->rx_pkt_count, 1lu);
    EXPECT_GT(ctx_s->rx_pkt_count, 1lu);

    client_zmq_rr_if.stop(ctx_s);
    client_zmq_rr_if.stop(ctx_c);
}
