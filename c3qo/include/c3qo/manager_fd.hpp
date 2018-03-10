#ifndef C3QO_MANAGER_FD_HPP
#define C3QO_MANAGER_FD_HPP

// C++ library headers
#include <vector>

// System library headers

// ZeroMQ header
#include <zmq.h>

// Routine to call when socket or file descriptor is ready
struct fd_call
{
    void *ctx;                                         // Context for the callback
    void (*callback)(void *ctx, int fd, void *socket); // Callback to call on a file descriptor or a socket
};

class manager_fd
{
  protected:
    std::vector<struct fd_call> callback_; // Callbacks for read and write events
    std::vector<zmq_pollitem_t> fd_;       // File descriptors or socket registered

  protected:
    int find(int fd, void *socket);

  public:
    bool add(void *ctx, void (*callback)(void *, int, void *), int fd, void *socket, bool read);
    void remove(int fd, void *socket, bool read);

  public:
    int poll_fd();
};

#endif // C3QO_MANAGER_FD_HPP
