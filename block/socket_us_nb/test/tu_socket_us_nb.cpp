//
// @brief Test unit for the unix-stream non-blocking sockets
//

// Project libraries
#include "block/client_us_nb.hpp"
#include "block/server_us_nb.hpp"
#include "c3qo/block.hpp"
#include "c3qo/manager.hpp"
#include "utils/logger.hpp"

// Gtest library
#include "gtest/gtest.h"

// Client and server shall be linked
extern struct bk_if client_us_nb_if;
extern struct bk_if server_us_nb_if;

// Managers shall be linked
extern struct manager *m;

class tu_socket_us_nb : public testing::Test
{
    void SetUp();
    void TearDown();
};

void tu_socket_us_nb::SetUp()
{
    LOGGER_OPEN("tu_socket_us_nb");
    logger_set_level(LOGGER_LEVEL_DEBUG);

    // Populate the managers
    m = new struct manager;
}

void tu_socket_us_nb::TearDown()
{
    // Remove every timer
    m->tm.clear();

    // Clear the managers
    delete m;

    LOGGER_CLOSE();
    logger_set_level(LOGGER_LEVEL_NONE);
}

//
// @brief Establish a regular connection between server and client
//          - start server
//          - start client
//          - wait for connection to be acknowledged
//
// By the way, we test the statistics retrieval
//
TEST_F(tu_socket_us_nb, connect)
{
    struct server_us_nb_ctx *ctx_s; // server context
    struct client_us_nb_ctx *ctx_c; // client context
    char stats[16];                 // buffer to retrieve statistics
    int fd_count;                   // count of file descriptor handled by the server

    ctx_s = (struct server_us_nb_ctx *)server_us_nb_if.init(1);
    ctx_c = (struct client_us_nb_ctx *)client_us_nb_if.init(2);

    server_us_nb_if.start(ctx_s);
    client_us_nb_if.start(ctx_c);

    // Trigger client connection to the server
    m->fd.select_fd();

    // Verify that server has a new client
    server_us_nb_if.get_stats(ctx_s, stats, sizeof(stats));
    fd_count = atoi(stats);
    EXPECT_EQ(fd_count, 2);

    client_us_nb_if.stop(ctx_c);
    server_us_nb_if.stop(ctx_s);
}

//
// @brief Establish several connections from client to server
//
TEST_F(tu_socket_us_nb, multi_connect)
{
    struct server_us_nb_ctx *ctx_s;     // server context
    struct client_us_nb_ctx *ctx_c[10]; // client context

    ctx_s = (struct server_us_nb_ctx *)server_us_nb_if.init(1);
    for (int i = 0; i < 10; i++)
    {
        ctx_c[i] = (struct client_us_nb_ctx *)client_us_nb_if.init(i + 2);
    }

    server_us_nb_if.start(ctx_s);
    for (int i = 0; i < 10; i++)
    {
        client_us_nb_if.start(ctx_c[i]);
    }

    // Wait for every connection to be acknowledged
    // A loop takes at least 10ms and reconnection timers are 100ms long
    for (int i = 0; i < 20; i++)
    {
        // Lookup for something on the socket and make timer expire
        m->fd.select_fd();
        m->tm.check_exp();
    }
    EXPECT_EQ(ctx_s->fd_count, 11);

    for (int i = 0; i < 10; i++)
    {
        EXPECT_EQ(ctx_c[i]->connected, true);
    }

    server_us_nb_if.stop(ctx_s);
    for (int i = 0; i < 10; i++)
    {
        client_us_nb_if.stop(ctx_c[i]);
    }
}

//
// @brief Establish an inversed connection between server and client
//          - start client
//          - start the server
//          - wait for connection to be acknowledged
//
// This should trigger the connection retry mechanism
//
TEST_F(tu_socket_us_nb, connect_retry)
{
    struct server_us_nb_ctx *ctx_s; // server context
    struct client_us_nb_ctx *ctx_c; // client context

    // Initialize client and server
    ctx_s = (struct server_us_nb_ctx *)server_us_nb_if.init(1);
    ctx_c = (struct client_us_nb_ctx *)client_us_nb_if.init(2);
    ASSERT_TRUE(ctx_s != NULL);
    ASSERT_TRUE(ctx_c != NULL);
    EXPECT_TRUE(ctx_c->connected == false);
    EXPECT_TRUE(ctx_s->fd_count == 0);

    // Start client
    client_us_nb_if.start(ctx_c);
    EXPECT_TRUE(ctx_c->connected == false);

    // Start server
    server_us_nb_if.start(ctx_s);
    EXPECT_TRUE(ctx_s->fd_count == 1);

    // Wait for every connection to be acknowledged
    // A loop takes at least 10ms and reconnection timers are 100ms long
    for (int i = 0; i < 11; i++)
    {
        // Lookup for something on the socket and make timer expire
        m->fd.select_fd();
        m->tm.check_exp();
    }
    EXPECT_EQ(ctx_s->fd_count, 2);

    server_us_nb_if.stop(ctx_s);
    client_us_nb_if.stop(ctx_c);
}

//
// @brief Send a message from client to server
//
TEST_F(tu_socket_us_nb, data)
{
    struct server_us_nb_ctx *ctx_s; // server context
    struct client_us_nb_ctx *ctx_c; // client context

    // Initialize client and server
    ctx_s = (struct server_us_nb_ctx *)server_us_nb_if.init(1);
    ctx_c = (struct client_us_nb_ctx *)client_us_nb_if.init(2);
    ASSERT_NE(ctx_s, (void *)NULL);
    ASSERT_NE(ctx_c, (void *)NULL);
    EXPECT_EQ(ctx_c->connected, false);
    EXPECT_EQ(ctx_s->fd_count, 0);

    // Bind client and server to 0
    server_us_nb_if.bind(ctx_s, 0, 0);
    client_us_nb_if.bind(ctx_c, 0, 0);

    // Connect client and server
    server_us_nb_if.start(ctx_s);
    client_us_nb_if.start(ctx_c);
    EXPECT_EQ(ctx_s->fd_count, 1);
    EXPECT_EQ(ctx_c->connected, true);
    m->fd.select_fd();
    EXPECT_EQ(ctx_c->connected, true);

    // Send data from client to server
    client_us_nb_if.tx(ctx_c, (void *)"hello world");
    EXPECT_EQ(ctx_s->rx_pkt_count, (size_t)0);
    m->fd.select_fd();
    EXPECT_EQ(ctx_s->rx_pkt_count, (size_t)1);

    // Send data from server to client
    server_us_nb_if.tx(ctx_s, (void *)"hello world");
    EXPECT_EQ(ctx_c->rx_pkt_count, (size_t)0);
    m->fd.select_fd();
    EXPECT_EQ(ctx_c->rx_pkt_count, (size_t)1);

    server_us_nb_if.stop(ctx_s);
    client_us_nb_if.stop(ctx_c);
}

//
// @brief Test error cases
//
TEST_F(tu_socket_us_nb, error)
{
    // Ignore error log as it's expected to have some
    logger_set_level(LOGGER_LEVEL_CRIT);

    // Bind client and server to 0
    server_us_nb_if.bind(NULL, 0, 0);
    client_us_nb_if.bind(NULL, 0, 0);

    // Connect client and server
    server_us_nb_if.start(NULL);
    client_us_nb_if.start(NULL);

    // Connect client and server
    server_us_nb_if.stop(NULL);
    client_us_nb_if.stop(NULL);

    // Connect client and server
    server_us_nb_if.tx(NULL, NULL);
    client_us_nb_if.tx(NULL, NULL);
}
