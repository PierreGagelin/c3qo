//
// @brief Test file for the block manager
//

#define LOGGER_TAG "[TU.engine.fd]"

// Project headers
#include "c3qo/manager.hpp"

// Gtest library
#include "gtest/gtest.h"

struct block_fd : block
{
    bool boule_;

    block_fd(struct manager *mgr) : block(mgr), boule_(false) {}

    virtual void on_fd_(struct file_desc &fd) override final
    {
        (void)fd;
        boule_ = true;
    }
};

// Derive from the manager_fd class
class tu_manager_fd : public testing::Test
{
    void SetUp();
    void TearDown();

  public:
    struct manager mgr_;
    struct block_fd bk_;

    tu_manager_fd() : bk_(&mgr_) {}
};

void tu_manager_fd::SetUp()
{
    LOGGER_OPEN("tu_manager_fd");
    logger_set_level(LOGGER_LEVEL_DEBUG);
}

void tu_manager_fd::TearDown()
{
    logger_set_level(LOGGER_LEVEL_NONE);
    LOGGER_CLOSE();
}

//
// @brief Test the file descriptor manager
//
TEST_F(tu_manager_fd, fd)
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
    struct file_desc file_d;
    file_d.bk = &bk_;
    file_d.fd = fd;
    file_d.socket = nullptr;
    file_d.read = true;
    file_d.write = false;
    EXPECT_EQ(mgr_.fd_add(file_d), true);

    // Write into the managed
    fprintf(file, "hello world!");

    // Verify something is ready to be read
    EXPECT_GT(mgr_.fd_poll(), 0);

    // Verify that the callback was executed
    EXPECT_EQ(bk_.boule_, true);

    // Clean the file descriptor manager
    mgr_.fd_remove(file_d);
    fclose(file);
}
