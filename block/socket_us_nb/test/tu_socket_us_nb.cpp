//
// @brief Test unit for the unix-stream non-blocking sockets
//

// Project libraries
#include "c3qo/manager.hpp"

// Gtest library
#include "gtest/gtest.h"

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

char *prepare_buf()
{
    char *buf;

    buf = new char[256];

    memset(buf, 0, 256u);
    strcpy(buf, "hello world");

    return buf;
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
    struct bk_server_us_nb server;
    struct bk_client_us_nb client;
    char stats[16]; // buffer to retrieve statistics
    int fd_count;   // count of file descriptor handled by the server

    server.init_();
    client.init_();

    server.start_();
    client.start_();

    // Trigger client connection to the server
    m->fd.poll_fd();

    // Verify that server has a new client
    server.get_stats_(stats, sizeof(stats));
    fd_count = atoi(stats);
    EXPECT_EQ(fd_count, 2);

    client.stop_();
    server.stop_();
}

//
// @brief Establish several connections from client to server
//
TEST_F(tu_socket_us_nb, multi_connect)
{
    struct bk_server_us_nb server;
    struct bk_client_us_nb client[10];

    server.init_();
    for (int i = 0; i < 10; i++)
    {
        client[i].init_();
    }

    server.start_();
    for (int i = 0; i < 10; i++)
    {
        client[i].start_();
    }

    // Wait for every connection to be acknowledged
    // A loop takes at least 10ms and reconnection timers are 100ms long
    for (int i = 0; i < 20; i++)
    {
        // Lookup for something on the socket and make timer expire
        m->fd.poll_fd();
        m->tm.check_exp();
    }
    EXPECT_EQ(static_cast<struct server_us_nb_ctx *>(server.ctx_)->fd_count, 11);

    for (int i = 0; i < 10; i++)
    {
        EXPECT_EQ(static_cast<struct client_us_nb_ctx *>(client[i].ctx_)->connected, true);
    }

    server.stop_();
    for (int i = 0; i < 10; i++)
    {
        client[i].stop_();
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
    struct bk_server_us_nb server;
    struct bk_client_us_nb client;

    // Initialize client and server
    server.init_();
    client.init_();
    ASSERT_NE(server.ctx_, nullptr);
    ASSERT_NE(client.ctx_, nullptr);
    EXPECT_TRUE(static_cast<struct client_us_nb_ctx *>(client.ctx_)->connected == false);
    EXPECT_TRUE(static_cast<struct server_us_nb_ctx *>(server.ctx_)->fd_count == 0);

    // Start client
    client.start_();
    EXPECT_TRUE(static_cast<struct client_us_nb_ctx *>(client.ctx_)->connected == false);

    // Start server
    server.start_();
    EXPECT_TRUE(static_cast<struct server_us_nb_ctx *>(server.ctx_)->fd_count == 1);

    // Wait for every connection to be acknowledged
    // A loop takes at least 10ms and reconnection timers are 100ms long
    for (int i = 0; i < 11; i++)
    {
        // Lookup for something on the socket and make timer expire
        m->fd.poll_fd();
        m->tm.check_exp();
    }
    EXPECT_EQ(static_cast<struct server_us_nb_ctx *>(server.ctx_)->fd_count, 2);

    server.stop_();
    client.stop_();
}

//
// @brief Send a message from client to server
//
TEST_F(tu_socket_us_nb, data)
{
    struct bk_server_us_nb server;
    struct bk_client_us_nb client;
    char *data = prepare_buf();

    // Initialize client and server
    server.id_ = 1;
    server.init_();
    client.id_ = 2;
    client.init_();
    ASSERT_NE(static_cast<struct server_us_nb_ctx *>(server.ctx_), nullptr);
    ASSERT_NE(static_cast<struct client_us_nb_ctx *>(client.ctx_), nullptr);
    EXPECT_EQ(static_cast<struct client_us_nb_ctx *>(client.ctx_)->connected, false);
    EXPECT_EQ(static_cast<struct server_us_nb_ctx *>(server.ctx_)->fd_count, 0);

    // Bind client and server to 0
    server.bind_(0, 0);
    client.bind_(0, 0);

    // Connect client and server
    server.start_();
    client.start_();
    EXPECT_EQ(static_cast<struct server_us_nb_ctx *>(server.ctx_)->fd_count, 1);
    EXPECT_EQ(static_cast<struct client_us_nb_ctx *>(client.ctx_)->connected, true);
    m->fd.poll_fd();
    EXPECT_EQ(static_cast<struct client_us_nb_ctx *>(client.ctx_)->connected, true);

    // Send data from client to server
    client.tx_(data);
    EXPECT_EQ(static_cast<struct server_us_nb_ctx *>(server.ctx_)->rx_pkt_count, 0u);
    m->fd.poll_fd();
    EXPECT_EQ(static_cast<struct server_us_nb_ctx *>(server.ctx_)->rx_pkt_count, 1u);

    // Send data from server to client
    server.tx_(data);
    EXPECT_EQ(static_cast<struct client_us_nb_ctx *>(client.ctx_)->rx_pkt_count, 0u);
    m->fd.poll_fd();
    EXPECT_EQ(static_cast<struct client_us_nb_ctx *>(client.ctx_)->rx_pkt_count, 1u);

    server.stop_();
    client.stop_();

    delete[] data;
}

//
// @brief Test error cases
//
TEST_F(tu_socket_us_nb, error)
{
    struct bk_server_us_nb server;
    struct bk_client_us_nb client;

    // Ignore error log as it's expected to have some
    logger_set_level(LOGGER_LEVEL_CRIT);

    // Bind client and server to 0
    server.bind_(0, 0);
    client.bind_(0, 0);

    // Connect client and server
    server.start_();
    client.start_();

    // Connect client and server
    server.stop_();
    client.stop_();

    // Connect client and server
    server.tx_(nullptr);
    client.tx_(nullptr);
}
