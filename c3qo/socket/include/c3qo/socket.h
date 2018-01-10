#ifndef C3QO_SOCKET_H
#define C3QO_SOCKET_H


#include <unistd.h>


void    c3qo_socket_set_nb(int fd);
ssize_t c3qo_socket_write_nb(int fd, const char *buff, size_t size);
ssize_t c3qo_socket_read_nb(int fd, char *buff, size_t size);


#endif


