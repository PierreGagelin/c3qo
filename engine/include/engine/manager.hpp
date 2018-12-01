#ifndef MANAGER_HPP
#define MANAGER_HPP

// Project headers
#include "engine/block.hpp"
#include "utils/logger.hpp"
#include "utils/socket.hpp"

// C++ headers
#include <forward_list>

bool operator<(const struct timespec &a, const struct timespec &b);

bool operator==(const struct timer &a, const struct timer &b);
bool operator<(const struct timer &a, const struct timer &b);

//
// @struct manager
//
// @brief The manager is responsible of some block lives and to
//        give them a platform for timer, file descriptors...
//
struct manager
{
    manager();
    ~manager();

    //
    // Blocks management
    //
    std::unordered_map<int, struct block *> bk_map_;

    bool block_add(int id, const char *type);
    bool block_start(int id);
    bool block_stop(int id);
    bool block_del(int id);
    bool block_conf(int id, char *conf);
    bool block_bind(int id, int port, int bk_id);
    struct block *block_get(int id);
    void block_clear();

    //
    // Configuration management
    //
    bool conf_exec_cmd(enum bk_cmd cmd, int id, char *arg);
    bool conf_parse_line(char *line);
    bool conf_parse(const char *filename);
    bool conf_parse_pb_cmd(const uint8_t *pb_cmd, size_t size);
    size_t conf_get(char *buf, size_t len);

    //
    // Timers management
    //
    std::forward_list<struct timer> tm_list_; // Timers sorted by expiration

    bool timer_add(const struct timer &tm);
    void timer_del(const struct timer &tm);
    void timer_check_exp();
    void timer_clear();

    //
    // File descriptors management
    //
    std::vector<struct file_desc> callback_; // Callbacks for read and write events
    std::vector<zmq_pollitem_t> fd_;         // File descriptors or socket registered

    int fd_find(int fd, void *socket) const;
    bool fd_add(const struct file_desc &fd);
    void fd_remove(const struct file_desc &fd);
    int fd_poll();
};

#endif // MANAGER_HPP
