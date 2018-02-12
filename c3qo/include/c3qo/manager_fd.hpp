#ifndef C3QO_MANAGER_FD_HPP
#define C3QO_MANAGER_FD_HPP

// C++ library headers
#include <vector>

// System library headers
extern "C" {
#include <sys/select.h> // select and associated definitions
}

// Routine to call on a fd
struct fd_call
{
    void *ctx;
    void (*callback)(void *ctx, int fd);
};

class manager_fd
{
  public:
    manager_fd();

  protected:
    // List of registered callbacks for read and write events
    std::vector<struct fd_call> list_r_;
    std::vector<struct fd_call> list_w_;

  protected:
    int prepare_set(fd_set *set_r, fd_set *set_w);

  public:
    bool add(void *ctx, int fd, void (*callback)(void *ctx, int fd), bool read);
    void remove(int fd, bool read);

  public:
    int select_fd();
};

#endif // C3QO_MANAGER_FD_HPP
