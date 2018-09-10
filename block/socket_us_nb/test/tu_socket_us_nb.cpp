//
// @brief Test unit for the unix-stream non-blocking sockets
//

#define LOGGER_TAG "[TU.socket_us_nb]"

// Project libraries
#include "c3qo/manager.hpp"

// Gtest library
#include "gtest/gtest.h"

class tu_socket_us_nb : public testing::Test
{
    void SetUp();
    void TearDown();

  public:
    struct manager mgr_;
};

void tu_socket_us_nb::SetUp()
{
    LOGGER_OPEN("tu_socket_us_nb");
    logger_set_level(LOGGER_LEVEL_DEBUG);
}

void tu_socket_us_nb::TearDown()
{
    // Remove every timer
    mgr_.timer_clear();

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
    struct bk_server_us_nb server(&mgr_);
    struct bk_client_us_nb client(&mgr_);
    char stats[16]; // buffer to retrieve statistics
    int fd_count;   // count of file descriptor handled by the server

    server.init_();
    client.init_();

    server.start_();
    client.start_();

    // Trigger client connection to the server
    mgr_.fd_poll();

    // Verify that server has a new client
    server.get_stats_(stats, sizeof(stats));
    fd_count = atoi(stats);
    EXPECT_EQ(fd_count, 1);

    client.stop_();
    server.stop_();
}

//
// @brief Establish several connections from client to server
//
TEST_F(tu_socket_us_nb, multi_connect)
{
    struct bk_server_us_nb server(&mgr_);
    struct bk_client_us_nb client[10] = {
        bk_client_us_nb(&mgr_),
        bk_client_us_nb(&mgr_),
        bk_client_us_nb(&mgr_),
        bk_client_us_nb(&mgr_),
        bk_client_us_nb(&mgr_),
        bk_client_us_nb(&mgr_),
        bk_client_us_nb(&mgr_),
        bk_client_us_nb(&mgr_),
        bk_client_us_nb(&mgr_),
        bk_client_us_nb(&mgr_)};

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
        mgr_.fd_poll();
        mgr_.timer_check_exp();
    }
    EXPECT_EQ(server.clients_.size(), 10u);

    for (int i = 0; i < 10; i++)
    {
        EXPECT_EQ(client[i].connected_, true);
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
    struct bk_server_us_nb server(&mgr_);
    struct bk_client_us_nb client(&mgr_);

    // Initialize client and server
    EXPECT_EQ(client.connected_, false);
    EXPECT_EQ(server.clients_.size(), 0u);

    // Start client
    client.start_();
    EXPECT_EQ(client.connected_, false);

    // Start server
    server.start_();
    EXPECT_EQ(server.clients_.size(), 0u);

    // Wait for every connection to be acknowledged
    // A loop takes at least 10ms and reconnection timers are 100ms long
    for (int i = 0; i < 11; i++)
    {
        // Lookup for something on the socket and make timer expire
        mgr_.fd_poll();
        mgr_.timer_check_exp();
    }
    EXPECT_EQ(server.clients_.size(), 1u);

    server.stop_();
    client.stop_();
}

//
// @brief Send a message from client to server
//
TEST_F(tu_socket_us_nb, data)
{
    struct bk_server_us_nb server(&mgr_);
    struct bk_client_us_nb client(&mgr_);
    char *data = prepare_buf();

    // Initialize client and server
    server.id_ = 1;
    client.id_ = 2;
    EXPECT_EQ(client.connected_, false);
    EXPECT_EQ(server.clients_.size(), 0u);

    // Bind client and server to 0
    server.bind_(0, 0);
    client.bind_(0, 0);

    // Connect client and server
    server.start_();
    client.start_();
    mgr_.fd_poll();

    // Send data from client to server
    client.tx_(data);
    EXPECT_EQ(server.rx_pkt_, 0u);
    mgr_.fd_poll();
    EXPECT_EQ(server.rx_pkt_, 1u);

    // Send data from server to client
    server.tx_(data);
    EXPECT_EQ(client.rx_pkt_, 0u);
    mgr_.fd_poll();
    EXPECT_EQ(client.rx_pkt_, 1u);

    server.stop_();
    client.stop_();

    delete[] data;
}

//
// @brief Test error cases
//
TEST_F(tu_socket_us_nb, error)
{
    struct bk_server_us_nb server(&mgr_);
    struct bk_client_us_nb client(&mgr_);

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
