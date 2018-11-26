//
// @brief Test file for the block manager
//

#define LOGGER_TAG "[TU.engine.fd]"

// Project headers
#include "c3qo/tu.hpp"

struct block_fd : block
{
    bool boule_;

    explicit block_fd(struct manager *mgr) : block(mgr), boule_(false) {}

    virtual void on_fd_(struct file_desc &fd) override final
    {
        (void)fd;
        boule_ = true;
    }
};

struct manager mgr_;

//
// @brief Test the file descriptor manager
//
static void tu_manager_fd_fd()
{
    struct block_fd bk_(&mgr_);
    char fname[] = "/tmp/tu_manager_fd.txt";
    FILE *file;
    int fd;

    // Open a file and get its file descriptor
    file = fopen(fname, "w+");
    ASSERT(file != nullptr);
    fd = fileno(file);
    ASSERT(fd != -1);

    // Add a file descriptor to be managed for reading
    struct file_desc file_d;
    file_d.bk = &bk_;
    file_d.fd = fd;
    file_d.socket = nullptr;
    file_d.read = true;
    file_d.write = false;
    ASSERT(mgr_.fd_add(file_d) == true);

    // Write into the managed
    fprintf(file, "hello world!");

    // Verify something is ready to be read
    ASSERT(mgr_.fd_poll() > 0);

    // Verify that the callback was executed
    ASSERT(bk_.boule_ == true);

    // Clean the file descriptor manager
    mgr_.fd_remove(file_d);
    fclose(file);
}

int main(int, char **)
{
    LOGGER_OPEN("tu_manager_fd");
    logger_set_level(LOGGER_LEVEL_DEBUG);

    tu_manager_fd_fd();

    LOGGER_CLOSE();
    return 0;
}
