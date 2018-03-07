#ifndef C3QO_MANAGER_FD_HPP
#define C3QO_MANAGER_FD_HPP

// C++ library headers
#include <vector>

// System library headers
extern "C" {
#include <sys/select.h> // select and associated definitions
}

// ZeroMQ header
#include <zmq.h>

// Routine to call when socket or file descriptor is ready
struct fd_call
{
    void *ctx;                           // Context for the callback
    void (*callback)(void *ctx, int fd); // Callback to call on event
};

class manager_fd
{
  protected:
    std::vector<struct fd_call> callback_; // Callbacks for read and write events
    std::vector<zmq_pollitem_t> fd_;       // File descriptors or socket registered

  protected:
    int find(int fd);
    int prepare_set(fd_set *set_r, fd_set *set_w);

  public:
    bool add(void *ctx, int fd, void (*callback)(void *ctx, int fd), bool read);
    void remove(int fd, bool read);

  public:
    int poll_fd();
};

#endif // C3QO_MANAGER_FD_HPP
