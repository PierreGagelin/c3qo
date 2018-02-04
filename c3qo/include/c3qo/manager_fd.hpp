#ifndef C3QO_MANAGER_FD_HPP
#define C3QO_MANAGER_FD_HPP

namespace manager_fd
{

// Initialize and clean the file descriptor manager
void init();
void clean();

// Add or remove a file descriptor
bool add(void *ctx, int fd, void (*callback)(void *ctx, int fd), bool read);
void remove(int fd, bool read);

// Lookup fd ready
int select();

} // END namespace manager_fd

#endif // C3QO_MANAGER_FD_HPP
