//
// @brief Test file for the block manager
//

// Project headers
#include "c3qo/manager.hpp"

// Gtest library
#include "gtest/gtest.h"

bool fd_called;
void fd_callback(void *ctx, int fd, void *socket)
{
    (void)ctx;
    (void)fd;
    (void)socket;
    fd_called = true;
}

// Derive from the manager_fd class
class tu_manager_fd : public testing::Test, public manager
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
    ASSERT_NE(file, nullptr);
    fd = fileno(file);
    ASSERT_NE(fd, -1);

    // Add a file descriptor to be managed for reading
    EXPECT_EQ(fd_add(nullptr, &fd_callback, fd, nullptr, true), true);

    // Write into the managed
    fprintf(file, "hello world!");

    // Verify something is ready to be read
    EXPECT_GT(fd_poll(), 0);

    // Verify that the callback was executed
    EXPECT_EQ(fd_called, true);

    // Clean the file descriptor manager
    fd_remove(fd, nullptr, true);
    fclose(file);
}
