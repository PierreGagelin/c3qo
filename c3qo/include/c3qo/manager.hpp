#ifndef C3QO_MANAGER_HPP
#define C3QO_MANAGER_HPP

#include "c3qo/block.hpp"

#define NSEC_MAX (1000 * 1000 * 1000) // Maximum number of nsec + 1

bool operator==(const struct timer &a, const struct timer &b);
bool operator<(const struct timer &a, const struct timer &b);

bool operator==(const struct timespec &a, const struct timespec &b);
bool operator<(const struct timespec &a, const struct timespec &b);

// Routine to call when socket or file descriptor is ready
struct fd_call
{
    void *ctx;                                         // Context for the callback
    void (*callback)(void *ctx, int fd, void *socket); // Callback to call on a file descriptor or a socket
};

//
// @struct manager
//
// @brief The manager is responsible of some block lives and to
//        give them a platform for timer, file descriptors...
//
struct manager
{
  public:
    manager();
    ~manager();

    //
    // Blocks management
    //
  protected:
    // Map of blocks
    std::unordered_map<int, struct block *> bk_map_;

  public:
    bool block_add(int id, const char *type);
    bool block_init(int id);
    bool block_conf(int id, char *conf);
    bool block_bind(int id, int port, int bk_id);
    bool block_start(int id);
    bool block_stop(int id);

  public:
    struct block *block_get(int id);
    void block_del(int id);
    void block_clear();

    //
    // Configuration management
    //
  public:
    bool conf_exec_cmd(enum bk_cmd cmd, int id, char *arg);
    bool conf_parse_line(char *line);
    bool conf_parse(const char *filename);
    size_t conf_get(char *buf, size_t len);

    //
    // Timers management
    //
  protected:
    // Singly-linked list of timers sorted by expiration
    std::forward_list<struct timer> tm_list_;

  public:
    bool timer_add(struct timer &tm);
    void timer_del(struct timer &tm);

  public:
    void timer_check_exp();

  public:
    void timer_clear();

    //
    // File descriptors management
    //
  protected:
    std::vector<struct file_desc> callback_; // Callbacks for read and write events
    std::vector<zmq_pollitem_t> fd_;         // File descriptors or socket registered

  protected:
    int fd_find(int fd, void *socket) const;

  public:
    bool fd_add(const struct file_desc &fd);
    void fd_remove(const struct file_desc &fd);

  public:
    int fd_poll();
};

#endif // C3QO_MANAGER_HPP
