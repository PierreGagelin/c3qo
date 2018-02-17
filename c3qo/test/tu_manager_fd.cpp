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
#include "c3qo/manager_fd.hpp"
#include "utils/logger.hpp"

// Gtest library
#include "gtest/gtest.h"

// Managers shall be linked
extern class manager_fd m_fd;

bool fd_called;
void fd_callback(void *ctx, int fd)
{
    (void)fd;
    (void)ctx;

    fd_called = true;
}

class tu_manager_fd : public testing::Test
{
  public:
    void SetUp();
    void TearDown();
};

void tu_manager_fd::SetUp()
{
    LOGGER_OPEN("tu_manager_fd");
    logger_set_level(LOGGER_LEVEL_DEBUG);

    fd_called = false;
}

void tu_manager_fd::TearDown()
{
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
    EXPECT_EQ(m_fd.add(NULL, fd, &fd_callback, true), true);

    // Write into the managed
    fprintf(file, "hello world!");

    // Verify something is ready to be read
    EXPECT_GT(m_fd.select_fd(), 0);

    // Verify that the callback was executed
    EXPECT_EQ(fd_called, true);

    // Clean the file descriptor manager
    m_fd.remove(fd, true);
    fclose(file);
}
