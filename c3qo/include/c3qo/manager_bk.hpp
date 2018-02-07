#ifndef C3QO_MANAGER_HPP
#define C3QO_MANAGER_HPP

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
// @struct command
//
// Generic command to manage a block
//
struct command
{
    int id;          // Identifier of the block
    enum bk_cmd cmd; // Type of command
    char arg[4096];  // Argument of the command
};

class manager_bk
{
  public:
    manager_bk();
    ~manager_bk();

  protected:
    // Map of blocks
    std::unordered_map<int, struct bk_info> bk_map_;

    // Command to execute on a block
    // Using only one instance to minimize stack pression
    struct command cmd_;

  protected:
    void block_add(int id, enum bk_type type);
    void block_init(struct bk_info &bki);
    void block_start(struct bk_info &bki);
    void block_stop(struct bk_info &bki);

  protected:
    void block_del(int id);
    void block_clear();

  public:
    int conf_parse_line(FILE *file);
    void exec_cmd();

  public:
    bool conf_parse(const char *filename);
    size_t conf_get(char *buf, size_t len);
};

#endif // C3QO_MANAGER_HPP
