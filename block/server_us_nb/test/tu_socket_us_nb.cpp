/**
 * @brief Test unit for the unix-stream non-blocking sockets
 */


#include "gtest/gtest.h"

extern "C"
{
#include "c3qo/block.h"      // BK_INIT, BK_START...
#include "c3qo/logger.h"     // LOGGER_OPEN, LOGGER_DEBUG...
#include "c3qo/manager_fd.h" // manager_fd_init
}


// TU should be linked with the block
extern struct bk_if client_us_nb_entry;
extern struct bk_if server_us_nb_entry;


class tu_socket_us_nb : public testing::Test
{
        void SetUp();
        void TearDown();
};

void tu_socket_us_nb::SetUp()
{
        LOGGER_OPEN();
        logger_set_level(LOGGER_LEVEL_MAX);
        LOGGER_DEBUG("/**** BEGIN TEST CASE ****/");

        manager_fd_init();
}

void tu_socket_us_nb::TearDown()
{
        LOGGER_DEBUG("/**** END TEST CASE ****/");
        LOGGER_CLOSE();
        logger_set_level(LOGGER_LEVEL_NONE);
}


/**
 * @brief Establish a connection between server and client
 *          - start server then client
 *          - wait for connection to be acknowledged
 *
 * @note Next step : reconnection when client is started first
 */
TEST_F(tu_socket_us_nb, connection)
{
        int fd_count; // count of file descriptor handled by the server

        server_us_nb_entry.ctrl(BK_INIT, NULL);
        client_us_nb_entry.ctrl(BK_INIT, NULL);

        server_us_nb_entry.ctrl(BK_START, NULL);
        client_us_nb_entry.ctrl(BK_START, NULL);

        do
        {
                char buf[16];

                manager_fd_select();

                server_us_nb_entry.stats(buf, 16);
                fd_count = atoi(buf);
        }
        while (fd_count < 2);

        server_us_nb_entry.ctrl(BK_STOP, NULL);
        client_us_nb_entry.ctrl(BK_STOP, NULL);
}


