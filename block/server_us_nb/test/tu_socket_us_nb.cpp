/**
 * @brief Test unit for the unix-stream non-blocking sockets
 */


#include "gtest/gtest.h"

extern "C"
{
#include <pthread.h>   // pthread

#include "c3qo/block.h"      // BK_INIT, BK_START...
#include "c3qo/logger.h"     // LOGGER_OPEN, LOGGER_DEBUG...
#include "c3qo/manager_fd.h" // manager_fd_init
}


#define CLIENT 0
#define SERVER 1


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


// Argument to thread_start()
struct thread_info
{
        pthread_t tid;
};

// States to synchronize threads
static bool server_started   = false;
static bool client_connected = false;


// Client thread connecting to server
static void * thread_socket_client(void *arg)
{
        (void) arg;

        LOGGER_DEBUG("Started client pthread");

        client_us_nb_entry.ctrl(BK_INIT, NULL);

        while (server_started == false)
        {
                // Wait for the server to be ready
        }
        client_us_nb_entry.ctrl(BK_START, NULL);

        while (client_connected == false)
        {
                // Wait for the server to receive connection
        }
        client_us_nb_entry.ctrl(BK_STOP, NULL);

        return NULL;
}


// Server thread waiting for a client
static void * thread_socket_server(void *arg)
{
        int fd_count; // Server file descriptor count

        (void) arg;

        LOGGER_DEBUG("Started server pthread");

        server_us_nb_entry.ctrl(BK_INIT, NULL);

        server_us_nb_entry.ctrl(BK_START, NULL);
        server_started = true;

        do
        {
                char buf[16];

                // Try to catch client connection
                manager_fd_select();

                // Get number of file descriptors
                server_us_nb_entry.stats(buf, 16);

                fd_count = atoi(buf);
        }
        while (fd_count < 2);

        client_connected = true;

        server_us_nb_entry.ctrl(BK_STOP, NULL);

        return NULL;
}


/**
 * @brief Establish a connection between server and client in two threads
 *        This is bad because code isn't thread-safe
 */
TEST_F(tu_socket_us_nb, connection_pthread)
{
        pthread_attr_t     attr[2];
        struct thread_info tinf[2];
        void               *res;

        // Start a thread for client and server
        ASSERT_TRUE(pthread_attr_init(&attr[CLIENT]) == 0);
        ASSERT_TRUE(pthread_attr_init(&attr[SERVER]) == 0);

        ASSERT_TRUE(pthread_create(&tinf[CLIENT].tid, &attr[CLIENT], &thread_socket_client, &tinf[CLIENT]) == 0);
        ASSERT_TRUE(pthread_create(&tinf[SERVER].tid, &attr[SERVER], &thread_socket_server, &tinf[SERVER]) == 0);

        // Don't need it anymore
        EXPECT_TRUE(pthread_attr_destroy(&attr[CLIENT]) == 0);
        EXPECT_TRUE(pthread_attr_destroy(&attr[SERVER]) == 0);

        // Join threads
        EXPECT_TRUE(pthread_join(tinf[CLIENT].tid, &res) == 0);
        EXPECT_TRUE(pthread_join(tinf[SERVER].tid, &res) == 0);
}


/**
 * @brief Establish a connection between server and client
 *          - start server then client
 *          - wait for connection to be acknowledged
 *
 * @note Next step : reconnection when client is started first
 */
TEST_F(tu_socket_us_nb, connection_monothread)
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


