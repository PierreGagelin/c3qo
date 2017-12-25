#ifndef C3QO_SOCKET_HPP
#define C3QO_SOCKET_HPP

// System library headers
extern "C" {
#include <sys/types.h>  // sockaddr, socklen_t
#include <sys/socket.h> // sockaddr, socklen_t
}

void c3qo_socket_set_nb(int fd);
int c3qo_socket_connect_nb(int fd, const struct sockaddr *addr, socklen_t len);
ssize_t c3qo_socket_write_nb(int fd, const char *buff, size_t size);
ssize_t c3qo_socket_read_nb(int fd, char *buff, size_t size);

#endif // C3QO_SOCKET_HPP
