//
// @brief Test unit for the unix-stream non-blocking sockets
//

// Project libraries
#include "block/client_us_nb.hpp"
#include "block/server_us_nb.hpp"
#include "c3qo/block.hpp"
#include "c3qo/manager_fd.hpp"
#include "c3qo/manager_tm.hpp"
#include "utils/logger.hpp"

// Gtest library
#include "gtest/gtest.h"

// Client and server shall be linked
extern struct bk_if client_us_nb_if;
extern struct bk_if server_us_nb_if;

// Managers shall be linked
extern class manager_tm m_tm;
extern class manager_fd m_fd;

class tu_socket_us_nb : public testing::Test
{
    void SetUp();
    void TearDown();
};

void tu_socket_us_nb::SetUp()
{
    LOGGER_OPEN();
    logger_set_level(LOGGER_LEVEL_DEBUG);
}

void tu_socket_us_nb::TearDown()
{
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
    int fd_count;                   // count of file descriptor handled by the server

    ctx_s = (struct server_us_nb_ctx *)server_us_nb_if.init(1);
    ctx_c = (struct client_us_nb_ctx *)client_us_nb_if.init(2);

    server_us_nb_if.start(ctx_s);
    client_us_nb_if.start(ctx_c);

    do
    {
        char buf[16];

        m_fd.select_fd();

        server_us_nb_if.get_stats(ctx_s, buf, 16);
        fd_count = atoi(buf);
    } while (fd_count < 2);

    server_us_nb_if.stop(ctx_s);
    client_us_nb_if.stop(ctx_c);
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
    ASSERT_TRUE(ctx_c != NULL);
    ASSERT_TRUE(ctx_c != NULL);
    EXPECT_TRUE(ctx_c->connected == false);
    EXPECT_TRUE(ctx_s->fd_count == 0);

    // Start client
    client_us_nb_if.start(ctx_c);
    EXPECT_TRUE(ctx_c->connected == false);

    // Start server
    server_us_nb_if.start(ctx_s);
    EXPECT_TRUE(ctx_s->fd_count == 1);

    do
    {
        // Lookup for something on the socket and make timer expire
        m_fd.select_fd();
        m_tm.check_exp();
    } while (ctx_s->fd_count < 2);

    server_us_nb_if.stop(ctx_s);
    client_us_nb_if.stop(ctx_c);
}
