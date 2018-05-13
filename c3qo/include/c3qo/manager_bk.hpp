#ifndef C3QO_MANAGER_BK_HPP
#define C3QO_MANAGER_BK_HPP

// C++ library headers
#include <boost/intrusive_ptr.hpp> // boost::intrusive_ptr
#include <cstdlib>                 // size_t, malloc, free, strtoul
#include <unordered_map>           // unordered_map container
#include <vector>                  // vector

// Project headers
#include "c3qo/block.hpp" // struct bk_if, enum bk_type, enum bk_state

#define MAX_NAME 32u

//
// @struct bind_info
//
// @brief Information to bind to blocks together
//
struct bind_info
{
    int port;                                  // Port from source block
    int bk_id;                                 // Identifier of the destination block
    boost::intrusive_ptr<class bk_info> block; // Destination block
};

//
// @class bk_info
//
// @brief Description of a block for the manager
//
class bk_info
{
  public:
    std::vector<struct bind_info> bind; // Block bindings
    struct bk_if *bk;                   // Block interface
    void *ctx;                          // Block context
    int id;                             // Block ID
    char type[MAX_NAME];                // Block type
    enum bk_state state;                // Block state

  public:
    // Intrusive pointer information
    int ref;
    bk_info() : bk(NULL), ctx(NULL), id(0), state(STATE_STOP), ref(0){};
};

// Implement the instrusive pointer take-release
inline void intrusive_ptr_add_ref(class bk_info *x)
{
    ++x->ref;
}
inline void intrusive_ptr_release(class bk_info *x)
{
    --x->ref;
    if (x->ref == 0)
    {
        delete x;
    }
}

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
    std::unordered_map<int, boost::intrusive_ptr<class bk_info> > bk_map_;

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
    const class bk_info *block_get(int id);
    void block_del(int id);
    void block_clear();

  public:
    bool exec_cmd(enum bk_cmd cmd, int id, char *arg);
    bool conf_parse_line(char *line);
    bool conf_parse(const char *filename);
    size_t conf_get(char *buf, size_t len);
};

#endif // C3QO_MANAGER_BK_HPP
