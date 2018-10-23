#ifndef C3QO_SOCKET_HPP
#define C3QO_SOCKET_HPP

// Project headers
#include "utils/include.hpp"

//
// @struct c3qo_zmq_part
//
// @brief ZMQ message part, a message can contain several
//
struct c3qo_zmq_part
{
    char *data;
    size_t len;
};

int socket_nb(int domain, int type, int protocol);
void socket_nb_set(int fd);
int socket_nb_connect(int fd, const struct sockaddr *addr, socklen_t len);
bool socket_nb_connect_check(int fd);

ssize_t socket_nb_write(int fd, const char *buff, size_t size);
ssize_t socket_nb_read(int fd, char *buff, size_t size);

void c3qo_zmq_msg_del(std::vector<struct c3qo_zmq_part> &msg);
void socket_zmq_read(void *socket, std::vector<struct c3qo_zmq_part> &msg, int flags = ZMQ_DONTWAIT);
bool socket_zmq_write(void *socket, std::vector<struct c3qo_zmq_part> &msg, int flags = ZMQ_DONTWAIT);
int socket_zmq_get_event(void *monitor);

#endif // C3QO_SOCKET_HPP
