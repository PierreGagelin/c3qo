#ifndef C3QO_MANAGER_FD_HPP
#define C3QO_MANAGER_FD_HPP

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
  protected:
    // Sets of file descriptors managed for read and write
    fd_set set_r;
    fd_set set_w;
    int set_max;

  protected:
    // List of registered callbacks for read and write events
    struct fd_call list_r[FD_SETSIZE];
    struct fd_call list_w[FD_SETSIZE];

  protected:
    void update_max();
    void prepare_set();

  public:
    bool add(void *ctx, int fd, void (*callback)(void *ctx, int fd), bool read);
    void remove(int fd, bool read);

  public:
    void init();
    void clean();

  public:
    int select_fd();
};

#endif // C3QO_MANAGER_FD_HPP
