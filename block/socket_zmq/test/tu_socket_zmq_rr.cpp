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
    struct server_zmq_rr_ctx *ctx_s; // server context
    struct client_zmq_rr_ctx *ctx_c; // client context

    // Initialize client and server
    ctx_s = (struct server_zmq_rr_ctx *)server_zmq_rr_if.init(1);
    ctx_c = (struct client_zmq_rr_ctx *)client_zmq_rr_if.init(2);
    ASSERT_NE(ctx_s, (void *)NULL);
    ASSERT_NE(ctx_c, (void *)NULL);

    // Connect client and server
    server_zmq_rr_if.start(ctx_s);
    client_zmq_rr_if.start(ctx_c);

    // Send data from client to server
    client_zmq_rr_if.tx(ctx_c, (void *)"hello world");
    EXPECT_EQ(ctx_s->rx_pkt_count, (unsigned long)0);
    m->fd.poll_fd();
    EXPECT_EQ(ctx_s->rx_pkt_count, (unsigned long)1);

    // Send data from server to client
    server_zmq_rr_if.tx(ctx_s, (void *)"hello world");
    EXPECT_EQ(ctx_c->rx_pkt_count, (unsigned long)0);
    m->fd.poll_fd();
    EXPECT_EQ(ctx_c->rx_pkt_count, (unsigned long)1);

    server_zmq_rr_if.stop(ctx_s);
    client_zmq_rr_if.stop(ctx_c);
}
