#ifndef SOCKET_HPP
#define SOCKET_HPP

// Project headers
#include "utils/include.hpp"

// ZMQ header
#include <zmq.h>

//
// @struct c3qo_zmq_part
//
// @brief ZMQ message part, a message can contain several of these
//
struct c3qo_zmq_part
{
    char *data;
    size_t len;
};

void c3qo_zmq_msg_del(std::vector<struct c3qo_zmq_part> &msg);
void socket_zmq_read(void *socket, std::vector<struct c3qo_zmq_part> &msg, int flags = ZMQ_DONTWAIT);
bool socket_zmq_write(void *socket, std::vector<struct c3qo_zmq_part> &msg, int flags = ZMQ_DONTWAIT);
int socket_zmq_get_event(void *monitor);

#endif // SOCKET_HPP
