//
// @brief Test unit for the unix-stream non-blocking sockets
//

// Project libraries
#include "c3qo/block.hpp"
#include "c3qo/manager_fd.hpp"
#include "utils/logger.hpp"

// Gtest library
#include "gtest/gtest.h"

// TU should be linked with the block
extern struct bk_if client_us_nb_if;
extern struct bk_if server_us_nb_if;

class tu_socket_us_nb : public testing::Test
{
    void SetUp();
    void TearDown();
};

void tu_socket_us_nb::SetUp()
{
    LOGGER_OPEN();
    logger_set_level(LOGGER_LEVEL_DEBUG);

    manager_fd::init();
}

void tu_socket_us_nb::TearDown()
{
    LOGGER_CLOSE();
    logger_set_level(LOGGER_LEVEL_NONE);
}

//
// @brief Establish a connection between server and client
//          - start server then client
//          - wait for connection to be acknowledged
//
// @note Next step : reconnection when client is started first
//
TEST_F(tu_socket_us_nb, connection)
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

        manager_fd::select();

        server_us_nb_if.get_stats(ctx_s, buf, 16);
        fd_count = atoi(buf);
    } while (fd_count < 2);

    server_us_nb_if.stop(ctx_s);
    client_us_nb_if.stop(ctx_c);
}
