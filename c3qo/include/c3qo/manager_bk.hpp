#ifndef C3QO_MANAGER_BK_HPP
#define C3QO_MANAGER_BK_HPP

// C++ library headers
#include <unordered_map> // unordered_map container
#include <cstdlib>       // size_t, malloc, free, strtoul

// Project headers
#include "c3qo/block.hpp" // struct bk_if, enum bk_type, enum bk_state

//
// @struct bk_info
//
// @brief Description of a block for the manager
//
struct bk_info
{
    struct bk_if bk;     // Block interface
    void *ctx;           // Block context
    int id;              // Block ID
    enum bk_type type;   // Block type
    enum bk_state state; // Block state
};

//
// @enum flow_type
//
enum flow_type
{
    FLOW_RX,    // RX flow
    FLOW_TX,    // TX flow
    FLOW_NOTIF, // Notification flow
};
const char *get_flow_type(enum flow_type type);

class manager_bk
{
  public:
    manager_bk();
    ~manager_bk();

  protected:
    // Map of blocks
    std::unordered_map<int, struct bk_info> bk_map_;

  protected:
    void block_flow(int id, void *data, enum flow_type type);

  public:
    void process_rx(int bk_id, void *data);
    void process_tx(int bk_id, void *data);
    void process_notif(int bk_id, void *notif);

  public:
    bool block_add(int id, enum bk_type type);
    bool block_init(int id);
    bool block_conf(int id, char *conf);
    bool block_bind(int id, int port, int bk_id);
    bool block_start(int id);
    bool block_stop(int id);

  public:
    const struct bk_info *block_get(int id);
    void block_del(int id);
    void block_clear();

  public:
    bool exec_cmd(enum bk_cmd cmd, int id, char *arg);
    bool conf_parse_line(char *line);
    bool conf_parse(const char *filename);
    size_t conf_get(char *buf, size_t len);
};

#endif // C3QO_MANAGER_BK_HPP
