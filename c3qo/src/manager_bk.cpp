

// C++ library headers
#include <cstdio> // fopen, fgets, sscanf

// Project headers
#include "c3qo/block.hpp"
#include "c3qo/manager_bk.hpp"
#include "utils/logger.hpp"

// Each block shall be linked
extern struct bk_if hello_if;
extern struct bk_if client_us_nb_if;
extern struct bk_if server_us_nb_if;

//
// @brief Constructor and destructor
//
manager_bk::manager_bk()
{
    memset(&cmd_, 0, sizeof(cmd_));
}
manager_bk::~manager_bk()
{
    block_clear();
}

//
// @brief Add or replace a block
//
// @param id   : Block ID
// @param type : Block type
//
void manager_bk::block_add(int id, enum bk_type type)
{
    std::pair<std::unordered_map<int, struct bk_info>::iterator, bool> ret;
    struct bk_info block;

    LOGGER_INFO("Add block [bk_id=%d ; bk_type=%s]", id, get_bk_type(type));

    switch (type)
    {
    case TYPE_HELLO:
    {
        block.bk = hello_if;
        break;
    }
    case TYPE_CLIENT_US_NB:
    {
        block.bk = client_us_nb_if;
        break;
    }
    case TYPE_SERVER_US_NB:
    {
        block.bk = server_us_nb_if;
        break;
    }
    default:
    {
        LOGGER_WARNING("Unknown block type value [bk_id=%d ; bk_type=%d]", id, type);
        return;
    }
    }

    block.id = id;
    block.type = type;
    block.state = STATE_STOP;

    block_del(id);
    bk_map_.insert({id, block});
}

//
// @brief Initialiaze a block
//
// @param bki : Block information
//
void manager_bk::block_init(struct bk_info &bki)
{
    // Verify block state
    switch (bki.state)
    {
    case STATE_STOP:
        // Normal case
        break;

    case STATE_INIT:
        block_start(bki);
        block_stop(bki);
        break;

    case STATE_START:
        block_stop(bki);
        break;
    }

    LOGGER_INFO("Initialize block [bk_id=%d ; bk_type=%s]", bki.id, get_bk_type(bki.type));

    bki.ctx = bki.bk.init(bki.id);
    bki.state = STATE_INIT;
}

//
// @brief Stop a block
//
// @param bki : Block information
//
void manager_bk::block_start(struct bk_info &bki)
{
    // Verify block state
    switch (bki.state)
    {
    case STATE_STOP:
        block_init(bki);
        break;

    case STATE_INIT:
        // Normal case
        break;

    case STATE_START:
        block_stop(bki);
        block_init(bki);
        break;
    }

    LOGGER_INFO("Start block [bk_id=%d ; bk_type=%s]", bki.id, get_bk_type(bki.type));

    bki.bk.start(bki.ctx);
    bki.state = STATE_START;
}

//
// @brief Stop a block
//
// @param bki : Block information
//
void manager_bk::block_stop(struct bk_info &bki)
{
    // Verify block state
    switch (bki.state)
    {
    case STATE_STOP:
        block_init(bki);
        block_start(bki);
        break;

    case STATE_INIT:
        block_start(bki);
        break;

    case STATE_START:
        // Normal case
        break;
    }

    LOGGER_INFO("Stop block [bk_id=%d ; bk_type=%s]", bki.id, get_bk_type(bki.type));

    bki.bk.stop(bki.ctx);
    bki.state = STATE_STOP;
}

//
// @brief Stop and delete a block
//
// @param id : Block ID
//
void manager_bk::block_del(int id)
{
    std::unordered_map<int, struct bk_info>::iterator it;

    it = bk_map_.find(id);
    if (it == bk_map_.end())
    {
        // Block does not exist
        return;
    }

    block_stop(it->second);

    LOGGER_INFO("Delete block [bk_id=%d ; bk_type=%s]", it->second.id, get_bk_type(it->second.type));

    bk_map_.erase(it);
}

//
// @brief Clean all blocks
//
void manager_bk::block_clear()
{
    std::unordered_map<int, struct bk_info>::iterator i;
    std::unordered_map<int, struct bk_info>::iterator e;

    // Stop every blocks
    i = bk_map_.begin();
    e = bk_map_.end();
    while (i != e)
    {
        block_stop(i->second);
        ++i;
    }

    bk_map_.clear();
}

//
// @brief Execute the global manager command
//        Some of them are directly executed by a block
//
void manager_bk::exec_cmd()
{
    std::unordered_map<int, struct bk_info>::iterator it;

    // Add a new block, the only command that does not require to find the block
    if (cmd_.cmd == CMD_ADD)
    {
        unsigned long bk_type;

        bk_type = strtoul(cmd_.arg, NULL, 10);

        block_add(cmd_.id, (enum bk_type)bk_type);

        return;
    }

    // Find the block concerned by the command
    it = bk_map_.find(cmd_.id);
    if (it == bk_map_.end())
    {
        LOGGER_WARNING("Cannot execute command on unknown block [bk_id=%d ; bk_cmd=%s ; cmd_arg=%s]", cmd_.id, get_bk_cmd(cmd_.cmd), cmd_.arg);
        return;
    }

    switch (cmd_.cmd)
    {
    case CMD_INIT:
    {
        block_init(it->second);
        break;
    }
    case CMD_CONF:
    {
        if (it->second.state == STATE_STOP)
        {
            block_init(it->second);
        }

        LOGGER_INFO("Configure block [bk_id=%d ; bk_type=%s ; conf=%s]", it->first, get_bk_type(it->second.type), cmd_.arg);

        it->second.bk.conf(it->second.ctx, cmd_.arg);
        break;
    }
    case CMD_BIND:
    {
        int port;
        int bk_id;
        int nb_arg;

        if (it->second.state == STATE_STOP)
        {
            block_init(it->second);
        }

        // Retrieve bindings parameters from command argument
        nb_arg = sscanf(cmd_.arg, "%d:%d", &port, &bk_id);
        if (nb_arg != 2)
        {
            LOGGER_WARNING("Cannot bind block: corrupted parameters [bk_id=%d ; cmd_arg=%s]", it->first, cmd_.arg);
            break;
        }

        LOGGER_INFO("Bind block [bk_id=%d ; bk_type=%s ; port=%d ; bk_id_dest=%d]", it->first, get_bk_type(it->second.type), port, bk_id);

        it->second.bk.bind(it->second.ctx, port, bk_id);
        break;
    }
    case CMD_START:
    {
        block_start(it->second);
        break;
    }
    case CMD_STOP:
    {
        block_stop(it->second);
        break;
    }
    default:
    {
        // Ignore this entry
        LOGGER_WARNING("Cannot execute unknown command [bk_id=%d ; bk_cmd=%d]", it->first, cmd_.cmd);
        break;
    }
    }
}

//
// @brief Get a configuration line and fill the global manager command
//
// @param file : configuration file
//
// @return Several values
//           - -1 on bad parsing
//           -  0 if there are no more lines
//           -  1 on success
//
int manager_bk::conf_parse_line(FILE *file)
{
    int nb_arg;

    if (feof(file) != 0)
    {
        LOGGER_DEBUG("Finished to read configuration file");
        return 0;
    }

    // Retrieve command
    nb_arg = fscanf(file, "%d %d %4095s\n", &cmd_.cmd, &cmd_.id, cmd_.arg);
    if (nb_arg != 3)
    {
        cmd_.arg[sizeof(cmd_.arg) - 1] = '\0';
        LOGGER_ERR("Corrupted configuration entry [bk_id=%d ; bk_cmd=%s ; bk_arg=%s]", cmd_.id, get_bk_cmd(cmd_.cmd), cmd_.arg);
        return -1;
    }

    return 1;
}

//
// @brief Parse a configuration file to retrieve block configuration
//
// @param filename : Name of the configuration file
//
bool manager_bk::conf_parse(const char *filename)
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
        if (rc == -1)
        {
            // Shouldn't read anymore
            ret = false;
            break;
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
size_t manager_bk::conf_get(char *buf, size_t len)
{
    std::unordered_map<int, struct bk_info>::const_iterator i;
    std::unordered_map<int, struct bk_info>::const_iterator e;
    size_t w;

    LOGGER_DEBUG("Getting blocks information")

    w = 0;
    i = bk_map_.begin();
    e = bk_map_.end();
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
