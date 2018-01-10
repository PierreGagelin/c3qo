#ifndef C3QO_MANAGER_FD_H
#define C3QO_MANAGER_FD_H


#include <stdbool.h>


/* Initialize and clean the file descriptor manager */
void manager_fd_init();
void manager_fd_clean();


/* Add or remove a file descriptor */
bool manager_fd_add(int fd, void (*callback) (int fd));
void manager_fd_remove(int fd);


/* Try to find fd ready for reading */
void manager_fd_select();


#endif /* C3QO_MANAGER_FD_H */


