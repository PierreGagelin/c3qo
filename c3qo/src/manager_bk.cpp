

// C++ library headers
#include <cerrno>  // errno
#include <cstdio>  // fopen, fgets, sscanf
#include <cstring> // memset, strerror

// System library headers
extern "C" {
#include <dlfcn.h> // dlopen, dlsym, dlerror
}

// Project headers
#include "c3qo/block.hpp"
#include "c3qo/manager_bk.hpp"
#include "utils/logger.hpp"

//
// @brief Convert flow type into a string
//
const char *get_flow_type(enum flow_type type)
{
    switch (type)
    {
    case FLOW_NOTIF:
        return "FLOW_TYPE_NOTIF";

    case FLOW_RX:
        return "FLOW_TYPE_RX";

    case FLOW_TX:
        return "FLOW_TYPE_TX";

    default:
        return "FLOW_TYPE_UNKNOWN";
    }
}

//
// @brief Constructor and destructor
//
manager_bk::manager_bk()
{
    // Nothing to do
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
bool manager_bk::block_add(int id, const char *type)
{
    struct bk_info block;
    void *self;
    int ret;

    // Retrieve block identifier
    if (id == 0)
    {
        LOGGER_ERR("Failed to add block: forbidden block ID [bk_id=%d ; bk_type=%s]", id, type);
        return false;
    }
    block.id = id;

    // Retrieve block type
    ret = snprintf(block.type, sizeof(block.type), "%s_if", type);
    if ((ret < 0) || ((size_t)ret >= sizeof(block.type)))
    {
        // snprintf failed or not enough room to append "_if"
        LOGGER_ERR("Failed to call snprintf [ret=%d ; max_len=%u]", ret, MAX_NAME - 4u);
        return false;
    }

    // Retrieve interface
    self = dlopen(NULL, RTLD_LAZY);
    if (self == NULL)
    {
        LOGGER_ERR("Failed to open ourselves: %s", dlerror());
        return false;
    }
    block.bk = (struct bk_if *)dlsym(self, block.type);
    if (block.bk == NULL)
    {
        LOGGER_ERR("Failed to find symbol: %s [symbol=%s]", dlerror(), block.type);
        dlclose(self);
        return false;
    }
    dlclose(self);

    block.state = STATE_STOP;

    LOGGER_INFO("Add block [bk_id=%d ; bk_type=%s]", block.id, block.type);

    block_del(id);
    bk_map_.insert({id, block});

    return true;
}

//
// @brief Initialiaze a block
//
// @param bki : Block information
//
bool manager_bk::block_init(int id)
{
    std::unordered_map<int, struct bk_info>::iterator it;

    // Find the block concerned by the command
    it = bk_map_.find(id);
    if (it == bk_map_.end())
    {
        LOGGER_WARNING("Cannot initialize block: unknown block ID [bk_id=%d]", id);
        return false;
    }

    // Verify block state
    switch (it->second.state)
    {
    case STATE_STOP:
        // Normal case
        break;

    case STATE_INIT:
    case STATE_START:
        LOGGER_WARNING("Cannot initialize block: block no stopped [bk_id=%d ; bk_state=%s]", id, get_bk_state(it->second.state));
        return false;
    }

    LOGGER_INFO("Initialize block [bk_id=%d ; bk_type=%s ; bk_state=%s]", it->second.id, it->second.type, get_bk_state(STATE_INIT));

    if (it->second.bk->init != NULL)
    {
        it->second.ctx = it->second.bk->init(it->second.id);
    }
    it->second.state = STATE_INIT;

    return true;
}

//
// @brief Configure a block
//
bool manager_bk::block_conf(int id, char *conf)
{
    std::unordered_map<int, struct bk_info>::iterator it;

    // Find the block concerned by the command
    it = bk_map_.find(id);
    if (it == bk_map_.end())
    {
        LOGGER_WARNING("Cannot configure block: unknown block ID [bk_id=%d]", id);
        return false;
    }

    if (it->second.state == STATE_STOP)
    {
        LOGGER_WARNING("Cannot configure block: block stopped [bk_id=%d]", id);
        return false;
    }

    LOGGER_INFO("Configure block [bk_id=%d ; bk_type=%s ; conf=%s]", id, it->second.type, conf);

    if (it->second.bk->conf != NULL)
    {
        it->second.bk->conf(it->second.ctx, conf);
    }

    return true;
}

//
// @brief Bind a block
//
bool manager_bk::block_bind(int id, int port, int bk_id)
{
    std::unordered_map<int, struct bk_info>::iterator it;

    // Find the block concerned by the command
    it = bk_map_.find(id);
    if (it == bk_map_.end())
    {
        LOGGER_WARNING("Cannot bind block: unknown block ID [bk_id=%d]", id);
        return false;
    }

    if (it->second.state == STATE_STOP)
    {
        LOGGER_WARNING("Cannot bind block: block stopped [bk_id=%d]", id);
        return false;
    }

    LOGGER_INFO("Bind block [bk_id=%d ; bk_type=%s ; port=%d ; bk_id_dest=%d]", it->first, it->second.type, port, bk_id);

    if (it->second.bk->bind != NULL)
    {
        it->second.bk->bind(it->second.ctx, port, bk_id);
    }

    return true;
}

//
// @brief Stop a block
//
// @param bki : Block information
//
bool manager_bk::block_start(int id)
{
    std::unordered_map<int, struct bk_info>::iterator it;

    // Find the block concerned by the command
    it = bk_map_.find(id);
    if (it == bk_map_.end())
    {
        LOGGER_WARNING("Cannot start block: unknown block ID [bk_id=%d]", id);
        return false;
    }

    // Verify block state
    switch (it->second.state)
    {
    case STATE_START:
    case STATE_STOP:
        LOGGER_WARNING("Cannot start block: block not initialized [bk_id=%d ; bk_state=%s]", id, get_bk_state(it->second.state));
        return false;

    case STATE_INIT:
        // Normal case
        break;
    }

    LOGGER_INFO("Start block [bk_id=%d ; bk_type=%s ; bk_state=%s]", it->second.id, it->second.type, get_bk_state(STATE_START));

    if (it->second.bk->start != NULL)
    {
        it->second.bk->start(it->second.ctx);
    }
    it->second.state = STATE_START;

    return true;
}

//
// @brief Stop a block
//
// @param bki : Block information
//
bool manager_bk::block_stop(int id)
{
    std::unordered_map<int, struct bk_info>::iterator it;

    // Find the block concerned by the command
    it = bk_map_.find(id);
    if (it == bk_map_.end())
    {
        LOGGER_WARNING("Cannot stop block: unknown block ID [bk_id=%d]", id);
        return false;
    }

    // Verify block state
    switch (it->second.state)
    {
    case STATE_INIT:
    case STATE_STOP:
        LOGGER_WARNING("Cannot stop block: block not started [bk_id=%d ; bk_state=%s]", id, get_bk_state(it->second.state));
        return false;

    case STATE_START:
        // Normal case
        break;
    }

    LOGGER_INFO("Stop block [bk_id=%d ; bk_type=%s ; bk_state=%s]", id, it->second.type, get_bk_state(STATE_STOP));

    if (it->second.bk->stop != NULL)
    {
        it->second.bk->stop(it->second.ctx);
    }
    it->second.state = STATE_STOP;

    return true;
}

//
// @brief Start a data flow between blocks
//
// @param bk_id : Block ID where the flow starts
// @param data  : Data to process
// @param type  : Flow type (RX, TX, NOTIF)
//
void manager_bk::block_flow(int bk_id, void *data, enum flow_type type)
{
    LOGGER_DEBUG("Begin data flow [bk_id=%d ; data=%p ; flow_type=%s]", bk_id, data, get_flow_type(type));

    // Process the data from one block to the other
    while (true)
    {
        const struct bk_info *bi;

        bi = block_get(bk_id);
        if (bi == NULL)
        {
            LOGGER_WARNING("Could not continue data flow: unknown block ID [bk_id=%d ; data=%p ; flow_type=%s]", bk_id, data, get_flow_type(type));
            return;
        }
        else if (bi->state != STATE_START)
        {
            LOGGER_WARNING("Could not continue data flow: block is not started [bk_id=%d ; bk_state=%s ; data=%p ; flow_type=%s]", bk_id, get_bk_state(bi->state), data, get_flow_type(type));
            return;
        }

        switch (type)
        {
        case FLOW_NOTIF:
            LOGGER_DEBUG("Notify block [bk_id=%d ; notif=%p]", bk_id, data);
            bk_id = bi->bk->ctrl(bi->ctx, data);
            break;

        case FLOW_RX:
            LOGGER_DEBUG("Process RX data [bk_id=%d ; data=%p]", bk_id, data);
            bk_id = bi->bk->rx(bi->ctx, data);
            break;

        case FLOW_TX:
            LOGGER_DEBUG("Process TX data [bk_id=%d ; data=%p]", bk_id, data);
            bk_id = bi->bk->tx(bi->ctx, data);
            break;

        default:
            LOGGER_WARNING("Could not continue data flow: unknown flow type value [bk_id=%d ; data=%p ; flow_type=%d]", bk_id, data, type);
            return;
        }

        if (bk_id == 0)
        {
            // Block ID 0 is the regular way out of this flow
            LOGGER_DEBUG("End data flow [bk_id=%d ; data=%p ; flow_type=%s]", bk_id, data, get_flow_type(type));
            break;
        }
    }
}

//
// @brief Send RX data to a block
//
// @param bk_id : Block ID
// @param data  : Data to process
//
void manager_bk::process_rx(int bk_id, void *data)
{
    block_flow(bk_id, data, FLOW_RX);
}

//
// @brief Send TX data to a block
//
// @param bk_id : Block ID
// @param data  : Data to process
//
void manager_bk::process_tx(int bk_id, void *data)
{
    block_flow(bk_id, data, FLOW_TX);
}

//
// @brief Send RX data to a block
//
// @param bk_id : Block ID
// @param notif : Notification to process
//
void manager_bk::process_notif(int bk_id, void *notif)
{
    block_flow(bk_id, notif, FLOW_NOTIF);
}

//
// @brief Get block information
//
const struct bk_info *manager_bk::block_get(int id)
{
    std::unordered_map<int, struct bk_info>::iterator it;

    it = bk_map_.find(id);
    if (it == bk_map_.end())
    {
        LOGGER_WARNING("Cannot get block: unknown block ID [bk_id=%d]", id);
        return NULL;
    }

    return &it->second;
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
        LOGGER_WARNING("Cannot delete block: unknown block ID [bk_id=%d]", id);
        return;
    }

    block_stop(id);

    LOGGER_INFO("Delete block [bk_id=%d ; bk_type=%s]", it->second.id, it->second.type);

    bk_map_.erase(it);
}

//
// @brief Clear all blocks
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
        block_stop(i->first);
        ++i;
    }

    bk_map_.clear();
}

//
// @brief Execute a block command
//
// @return True on success
//
bool manager_bk::exec_cmd(enum bk_cmd cmd, int id, char *arg)
{
    LOGGER_DEBUG("Execute block command [bk_cmd=%s ; bk_id=%d ; bk_arg=%s]", get_bk_cmd(cmd), id, arg);

    switch (cmd)
    {
    case CMD_ADD:
    {
        if ((arg == NULL) || ((arg = strtok(arg, " ")) == 0))
        {
            LOGGER_ERR("Failed to add block: block type required as third argument");
            return false;
        }

        return block_add(id, arg);
    }
    case CMD_INIT:
    {
        return block_init(id);
    }
    case CMD_CONF:
    {
        if (arg == NULL)
        {
            LOGGER_ERR("Failed to configure block: configuration entry required as third argument");
            return false;
        }

        return block_conf(id, arg);
    }
    case CMD_BIND:
    {
        int port;
        int dest;
        int nb_arg;

        if (arg == NULL)
        {
            LOGGER_DEBUG("Failed to bind block: configuration entry required as third argument");
            return false;
        }

        // Retrieve bindings parameters from command argument
        nb_arg = sscanf(arg, "%d:%d", &port, &dest);
        if (nb_arg != 2)
        {
            LOGGER_ERR("Failed to bind block: corrupted binding parameters [entry=%s]", arg);
            return false;
        }

        return block_bind(id, port, dest);
    }
    case CMD_START:
    {
        return block_start(id);
    }
    case CMD_STOP:
    {
        return block_stop(id);
    }
    default:
        // Ignore this entry
        LOGGER_WARNING("Cannot execute block command: unknown command value [bk_cmd=%d]", cmd);
        return false;
    }

    return true;
}

//
// @brief Parse a configuration line
//
// @return True on success
//
bool manager_bk::conf_parse_line(char *line)
{
    char *token;
    int id;
    enum bk_cmd cmd;
    char *arg;

    LOGGER_DEBUG("Parsing configuration line [line=%s]", line);

    // Retrieve command
    token = strtok(line, " ");
    if (token == NULL)
    {
        LOGGER_ERR("Failed to parse configuration line: no command");
        return false;
    }
    errno = 0;
    cmd = (enum bk_cmd)strtol(token, NULL, 10);
    if (errno != 0)
    {
        LOGGER_ERR("Failed to parse configuration line: command should be a decimal integer [bk_cmd=%s]", token);
        return false;
    }

    // Retrieve block identifier
    token = strtok(NULL, " ");
    if (token == NULL)
    {
        LOGGER_ERR("Failed to parse configuration line: no block identifier");
        return false;
    }
    errno = 0;
    id = (enum bk_cmd)strtol(token, NULL, 10);
    if (errno != 0)
    {
        LOGGER_ERR("Failed to parse configuration line: block identifier should be a decimal integer [bk_id=%s]", token);
        return false;
    }

    // Retrieve argument (optional)
    arg = strtok(NULL, "\0");

    return exec_cmd(cmd, id, arg);
}

//
// @brief Parse a configuration file to retrieve block configuration
//
// @param filename : Name of the configuration file
//
bool manager_bk::conf_parse(const char *filename)
{
    FILE *file;
    bool conf_success;

    file = fopen(filename, "r");
    if (file == NULL)
    {
        LOGGER_ERR("Couldn't open configuration file [filename=%s]", filename);
        return false;
    }
    LOGGER_INFO("Reading configuration file [filename=%s]", filename);

    // Read the configuration file
    conf_success = true;
    while (true)
    {
        char *line;
        size_t len;
        ssize_t written;

        // Read a line
        line = NULL;
        written = getline(&line, &len, file);
        if (written == -1)
        {
            // Check if it's an error or end of file
            if (feof(file) == 0)
            {
                LOGGER_ERR("Failed to call getline: %s [errno=%d]", strerror(errno), errno);
                conf_success = false;
            }
            else
            {
                LOGGER_DEBUG("Finished to read configuration file");
            }

            free(line);
            break;
        }
        else
        {
            line[written - 1] = '\0'; // Removes the '\n'

            // Parse the line
            if (conf_parse_line(line) == false)
            {
                // One corrupted entry, but the next ones can be good
                conf_success = false;
            }
        }

        free(line);
    }

    fclose(file);

    if (conf_success == true)
    {
        LOGGER_INFO("Configuration OK");
    }
    else
    {
        LOGGER_ERR("Configuration KO");
    }

    return conf_success;
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

        ret = snprintf(buf + w, len - w, "%u %s %d;", i->first, i->second.type, i->second.state);
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
