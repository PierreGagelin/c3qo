#ifndef C3QO_MANAGER_BK_HPP
#define C3QO_MANAGER_BK_HPP

// Project headers
#include "block/hello.hpp"
#include "block/client_us_nb.hpp"
#include "block/server_us_nb.hpp"
#include "block/zmq_pair.hpp"
#include "block/project_euler.hpp"
#include "block/trans_pb.hpp"
#include "c3qo/block.hpp"

//
// @enum flow_type
//
enum flow_type
{
    FLOW_RX,    // RX flow
    FLOW_TX,    // TX flow
    FLOW_NOTIF, // Notification flow
};
const char *flow_type_to_string(enum flow_type type);

class manager_bk
{
  public:
    manager_bk();
    ~manager_bk();

  protected:
    // Map of blocks
    std::unordered_map<int, struct block *> bk_map_;

  protected:
    void block_flow(int id, int port, void *data, enum flow_type type);

  public:
    void process_rx(int bk_id, int port, void *data);
    void process_tx(int bk_id, int port, void *data);
    void process_notif(int bk_id, int port, void *notif);

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

  public:
    bool exec_cmd(enum bk_cmd cmd, int id, char *arg);
    bool conf_parse_line(char *line);
    bool conf_parse(const char *filename);
    size_t conf_get(char *buf, size_t len);
};

#endif // C3QO_MANAGER_BK_HPP
