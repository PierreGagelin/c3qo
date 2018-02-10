#ifndef C3QO_SOCKET_HPP
#define C3QO_SOCKET_HPP

// System library headers
extern "C" {
#include <sys/types.h>  // sockaddr, socklen_t
#include <sys/socket.h> // sockaddr, socklen_t
}

int socket_nb(int domain, int type, int protocol);
void socket_nb_set(int fd);
int socket_nb_connect(int fd, const struct sockaddr *addr, socklen_t len);
bool socket_nb_connect_check(int fd);

ssize_t socket_nb_write(int fd, const char *buff, size_t size);
ssize_t socket_nb_read(int fd, char *buff, size_t size);

#endif // C3QO_SOCKET_HPP
