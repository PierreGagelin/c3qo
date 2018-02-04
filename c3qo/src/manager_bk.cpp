

// C++ library headers
#include <unordered_map> // unordered_map container
#include <cstdio>        // fopen, fgets, sscanf
#include <cstdlib>       // malloc, free, strtoul
#include <cstdint>       // fixed-size data types

// Project headers
#include "c3qo/block.hpp"
#include "c3qo/manager_bk.hpp"
#include "utils/logger.hpp"

// Each block shall be linked
extern struct bk_if hello_entry;
extern struct bk_if goodbye_entry;
extern struct bk_if client_us_nb_entry;
extern struct bk_if server_us_nb_entry;

namespace manager_bk
{

//
// @struct bk_info
//
// @brief Description of a block for the manager
//
struct bk_info
{
    struct bk_if bk;     // block interface
    enum bk_type type;   // type of the block
    enum bk_state state; // state of the block
};

// Map of blocks
std::unordered_map<int, struct bk_info> bk_map_;

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

// Using only one instance to minimize stack pression
struct command cmd_;

//
// @brief Create a block interface
//
// @param id   : Identifier in the block list
// @param type : Type of interface to implement
//
void block_add(int id, enum bk_type type)
{
    std::pair<std::unordered_map<int, struct bk_info>::iterator, bool> ret;
    struct bk_info block;

    LOGGER_DEBUG("Add block [bk_id=%d ; bk_type=%d]", id, type);

    switch (type)
    {
    case BK_TYPE_HELLO:
    {
        block.bk = hello_entry;
        break;
    }
    case BK_TYPE_GOODBYE:
    {
        block.bk = goodbye_entry;
        break;
    }
    case BK_TYPE_CLIENT_US_NB:
    {
        block.bk = client_us_nb_entry;
        break;
    }
    case BK_TYPE_SERVER_US_NB:
    {
        block.bk = server_us_nb_entry;
        break;
    }
    default:
    {
        LOGGER_WARNING("Unknown block type [bk_id=%d ; bk_type=%d]", id, type);
        return;
    }
    }

    block.type = type;
    block.state = BK_STATE_STOP;

    manager_bk::bk_map_[id] = block;
}

//
// @brief Execute the global manager command
//        Some of them are directly executed by a block
//
void exec_cmd()
{
    switch (manager_bk::cmd_.cmd)
    {
    case BK_CMD_ADD:
    {
        unsigned long bk_type;

        bk_type = strtoul(manager_bk::cmd_.arg, NULL, 10);

        block_add(manager_bk::cmd_.id, (enum bk_type)bk_type);

        break;
    }
    case BK_CMD_INIT:
    case BK_CMD_CONF:
    case BK_CMD_BIND:
    case BK_CMD_START:
    case BK_CMD_STOP:
    {
        std::unordered_map<int, struct bk_info>::const_iterator it;

        it = manager_bk::bk_map_.find(manager_bk::cmd_.id);
        if (it == manager_bk::bk_map_.end())
        {
            LOGGER_WARNING("Cannot stop unknown block [bk_id=%d ; bk_cmd=%d]", manager_bk::cmd_.id, manager_bk::cmd_.cmd);
            break;
        }

        it->second.bk.ctrl(manager_bk::cmd_.cmd, manager_bk::cmd_.arg);
        break;
    }
    default:
    {
        // Ignore this entry
        LOGGER_WARNING("Unknown block command [bk_cmd=%d]", manager_bk::cmd_.cmd);
    }
    }
}

//
// @brief Get a configuration line and fill the global manager command
//
// @param file : configuration file
//
// @return Several values
//           - -2 on bad parsing
//           - -1 on bad values
//           -  0 if there are no more lines
//           -  1 on success
//
int conf_parse_line(FILE *file)
{
    int nb_arg;
    int cmd;
    int id;

    if (feof(file) != 0)
    {
        LOGGER_DEBUG("Finished to read configuration file");
        return 0;
    }

    nb_arg = fscanf(file, "%d %d %4095s\n", &cmd, &id, manager_bk::cmd_.arg);
    if (nb_arg != 3)
    {
        manager_bk::cmd_.arg[sizeof(manager_bk::cmd_.arg) - 1] = '\0';
        LOGGER_ERR("Corrupted configuration entry [bk_id=%d ; bk_cmd=%d ; bk_arg=%s]", id, cmd, manager_bk::cmd_.arg);
        return -2;
    }

    // Check values (shouldn't stop the configuration)
    if ((cmd > BK_CMD_MAX) || (cmd <= BK_CMD_NONE))
    {
        LOGGER_WARNING("Block command doesn't exist [bk_id=%d, bk_cmd=%d]", id, cmd);
        return -1;
    }

    manager_bk::cmd_.cmd = (enum bk_cmd)cmd;
    manager_bk::cmd_.id = id;

    return 1;
}

//
// @brief Clean all blocks
//
void block_clean()
{
    manager_bk::bk_map_.clear();
}

//
// @brief Fill a string representing existing blocks
//        Format for each entry follows :
//          - <bk_id> <bk_type> <bk_state>;
//
// @param buf : string to fill
// @param len : maximum length to write
//
// @return actual length written
//
size_t conf_get(char *buf, size_t len)
{
    std::unordered_map<int, struct bk_info>::const_iterator i;
    std::unordered_map<int, struct bk_info>::const_iterator e;
    size_t w;

    LOGGER_DEBUG("Getting blocks information")

    w = 0;
    i = manager_bk::bk_map_.begin();
    e = manager_bk::bk_map_.end();
    while (i != e)
    {
        int ret;

        ret = snprintf(buf + w, len - w, "%u %d %d;", i->first, i->second.type, i->second.state);
        if (ret < 0)
        {
            LOGGER_ERR("snprintf failed");
            break;
        }
        else
        {
            w += (size_t)ret;
        }

        // Stop if there's no more room in buf
        if (w >= len)
        {
            LOGGER_WARNING("Not enough space to dump blocks information");
            w = len;
            break;
        }

        ++i;
    }

    return w;
}

//
// @brief Parse a configuration file to retrieve block configuration
//
// @param filename : Name of the configuration file
//
bool conf_parse(const char *filename)
{
    FILE *file;
    bool ret;

    file = fopen(filename, "r");
    if (file == NULL)
    {
        LOGGER_ERR("Couldn't open configuration file [filename=%s]", filename);
        return false;
    }
    LOGGER_INFO("Reading configuration file [filename=%s]", filename);

    // Read one line at a time
    ret = true;
    while (true)
    {
        int rc;

        rc = conf_parse_line(file);
        if (rc == -2)
        {
            // Shouldn't read anymore
            ret = false;
            break;
        }
        else if (rc == -1)
        {
            // Shouldn't use values
            continue;
        }
        else if (rc == 0)
        {
            // Finished to read file
            break;
        }

        exec_cmd();
    }

    fclose(file);

    if (ret == true)
    {
        LOGGER_INFO("Configuration OK");
    }
    else
    {
        LOGGER_ERR("Configuration KO");
    }

    return ret;
}

} // END namespace manager_bk
