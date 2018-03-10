//
// @brief Test file for the block manager
//

// C++ library headers
#include <stdio.h> // fopen, fileno, fprintf

// System library headers
extern "C" {
#include <unistd.h> // sleep
}

// Project headers
#include "c3qo/block.hpp"
#include "c3qo/manager.hpp"
#include "utils/logger.hpp"

// Gtest library
#include "gtest/gtest.h"

// Managers shall be linked
extern struct manager *m;

bool fd_called;
void fd_callback(void *ctx, int fd, void *socket)
{
    fd_called = true;
}

// Derive from the manager_fd class
class tu_manager_fd : public testing::Test, public manager_fd
{
  public:
    void SetUp();
    void TearDown();
};

void tu_manager_fd::SetUp()
{
    LOGGER_OPEN("tu_manager_fd");
    logger_set_level(LOGGER_LEVEL_DEBUG);

    // Populate the managers
    m = new struct manager;

    fd_called = false;
}

void tu_manager_fd::TearDown()
{
    // Clear the managers
    delete m;
    
    logger_set_level(LOGGER_LEVEL_NONE);
    LOGGER_CLOSE();
}

//
// @brief Test the file descriptor manager
//
TEST_F(tu_manager_fd, manager_fd)
{
    char fname[] = "/tmp/tu_manager_fd.txt";
    FILE *file;
    int fd;

    // Open a file and get its file descriptor
    file = fopen(fname, "w+");
    ASSERT_NE(file, (void *)NULL);
    fd = fileno(file);
    ASSERT_NE(fd, -1);

    // Add a file descriptor to be managed for reading
    EXPECT_EQ(add(NULL, &fd_callback, fd, NULL, true), true);

    // Write into the managed
    fprintf(file, "hello world!");

    // Verify something is ready to be read
    EXPECT_GT(poll_fd(), 0);

    // Verify that the callback was executed
    EXPECT_EQ(fd_called, true);

    // Clean the file descriptor manager
    remove(fd, NULL, true);
    fclose(file);
}
