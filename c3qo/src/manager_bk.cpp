

// Project headers
#include "c3qo/manager_bk.hpp"

//
// @brief Convert flow type into a string
//
const char *flow_type_to_string(enum flow_type type)
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
// @brief Retrieve a block interface pointer
//
static struct block *get_bk_if(const char *b)
{
    if (strcmp(b, "hello") == 0)
    {
        return new struct bk_hello;
    }
    if (strcmp(b, "client_us_nb") == 0)
    {
        return new struct bk_client_us_nb;
    }
    if (strcmp(b, "server_us_nb") == 0)
    {
        return new struct bk_server_us_nb;
    }
    if (strcmp(b, "zmq_pair") == 0)
    {
        return new struct bk_zmq_pair;
    }
    if (strcmp(b, "project_euler") == 0)
    {
        return new struct bk_project_euler;
    }
    if (strcmp(b, "trans_pb") == 0)
    {
        return new struct bk_trans_pb;
    }
    return nullptr;
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
    struct block *bk;

    // Retrieve block identifier
    if (id == 0)
    {
        LOGGER_ERR("Failed to add block: forbidden block ID [bk_id=%d ; bk_type=%s]", id, type);
        return false;
    }

    bk = get_bk_if(type);
    if (bk == nullptr)
    {
        LOGGER_ERR("Failed to find block interface [name=%s]", type);
        return false;
    }

    bk->id_ = id;
    bk->type_ = type;
    bk->state_ = STATE_STOP;

    LOGGER_INFO("Add block [bk_id=%d ; bk_type=%s]", bk->id_, bk->type_.c_str());

    block_del(id);
    bk_map_.insert({id, bk});

    return true;
}

//
// @brief Initialize a block
//
bool manager_bk::block_init(int id)
{
    // Find the block concerned by the command
    const auto &it = bk_map_.find(id);
    if (it == bk_map_.end())
    {
        LOGGER_WARNING("Cannot initialize block: unknown block ID [bk_id=%d]", id);
        return false;
    }

    // Verify block state
    switch (it->second->state_)
    {
    case STATE_STOP:
        // Normal case
        break;

    default:
        LOGGER_WARNING("Cannot initialize block: block not stopped [bk_id=%d ; bk_state=%s]", id, bk_state_to_string(it->second->state_));
        return false;
    }

    LOGGER_INFO("Initialize block [bk_id=%d ; bk_type=%s ; bk_state=%s]", it->second->id_, it->second->type_.c_str(), bk_state_to_string(STATE_INIT));

    it->second->id_ = id;
    it->second->state_ = STATE_INIT;

    it->second->init_();

    return true;
}

//
// @brief Configure a block
//
bool manager_bk::block_conf(int id, char *conf)
{
    // Find the block concerned by the command
    const auto &it = bk_map_.find(id);
    if (it == bk_map_.end())
    {
        LOGGER_WARNING("Cannot configure block: unknown block ID [bk_id=%d]", id);
        return false;
    }

    if (it->second->state_ == STATE_STOP)
    {
        LOGGER_WARNING("Cannot configure block: block stopped [bk_id=%d]", id);
        return false;
    }

    LOGGER_INFO("Configure block [bk_id=%d ; bk_type=%s ; conf=%s]", id, it->second->type_.c_str(), conf);


    it->second->conf_(conf);

    return true;
}

//
// @brief Bind a block
//
bool manager_bk::block_bind(int id, int port, int bk_id)
{
    struct bind_info bind;

    // Find the block concerned by the command
    const auto &source = bk_map_.find(id);
    const auto &end = bk_map_.end();
    if (source == end)
    {
        LOGGER_WARNING("Cannot bind block: unknown source [bk_id=%d]", id);
        return false;
    }

    if (source->second->state_ == STATE_STOP)
    {
        LOGGER_WARNING("Cannot bind block: block stopped [bk_id=%d]", id);
        return false;
    }

    LOGGER_INFO("Bind block [bk_id=%d ; port=%d ; bk_id_dest=%d]", source->first, port, bk_id);

    source->second->bind_(port, bk_id);

    // Add a binding in the engine with a weak reference to the block
    // If the block does not exist, we just let it undefined
    const auto &dest = bk_map_.find(bk_id);
    if (dest != end)
    {
        bind.bk = dest->second;
    }
    bind.port = port;
    bind.bk_id = bk_id;
    source->second->binds_.push_back(bind);

    return true;
}

//
// @brief Stop a block
//
bool manager_bk::block_start(int id)
{
    // Find the block concerned by the command
    const auto &it = bk_map_.find(id);
    if (it == bk_map_.end())
    {
        LOGGER_WARNING("Cannot start block: unknown block ID [bk_id=%d]", id);
        return false;
    }

    // Verify block state
    switch (it->second->state_)
    {
    case STATE_INIT:
        // Normal case
        break;

    default:
        LOGGER_WARNING("Cannot start block: block not initialized [bk_id=%d ; bk_state=%s]", id, bk_state_to_string(it->second->state_));
        return false;
    }

    LOGGER_INFO("Start block [bk_id=%d ; bk_type=%s ; bk_state=%s]", it->second->id_, it->second->type_.c_str(), bk_state_to_string(STATE_START));

    it->second->state_ = STATE_START;

    it->second->start_();

    return true;
}

//
// @brief Stop a block
//
bool manager_bk::block_stop(int id)
{
    // Find the block concerned by the command
    const auto &it = bk_map_.find(id);
    if (it == bk_map_.end())
    {
        LOGGER_WARNING("Cannot stop block: unknown block ID [bk_id=%d]", id);
        return false;
    }

    // Verify block state
    switch (it->second->state_)
    {
    case STATE_START:
        // Normal case
        break;

    default:
        LOGGER_WARNING("Cannot stop block: block not started [bk_id=%d ; bk_state=%s]", id, bk_state_to_string(it->second->state_));
        return false;
    }

    LOGGER_INFO("Stop block [bk_id=%d ; bk_type=%s ; bk_state=%s]", id, it->second->type_.c_str(), bk_state_to_string(STATE_STOP));

    it->second->state_ = STATE_STOP;

    it->second->stop_();

    return true;
}

//
// @brief Get block information
//
struct block *manager_bk::block_get(int id)
{
    const auto &it = bk_map_.find(id);
    if (it == bk_map_.cend())
    {
        return nullptr;
    }

    return it->second;
}

//
// @brief Stop and delete a block
//
// @param id : Block ID
//
void manager_bk::block_del(int id)
{
    const auto &it = bk_map_.find(id);
    if (it == bk_map_.end())
    {
        // Block does not exist, nothing to do
        return;
    }

    block_stop(it->first);
    delete it->second;

    LOGGER_INFO("Delete block [bk_id=%d ; bk_type=%s]", it->second->id_, it->second->type_.c_str());

    bk_map_.erase(it);
}

//
// @brief Clear all blocks
//
void manager_bk::block_clear()
{
    // Stop every blocks
    for (const auto &it : bk_map_)
    {
        block_stop(it.first);
        delete it.second;
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
    LOGGER_DEBUG("Execute block command [bk_cmd=%s ; bk_id=%d ; bk_arg=%s]", bk_cmd_to_string(cmd), id, arg);

    switch (cmd)
    {
    case CMD_ADD:
    {
        if ((arg == nullptr) || ((arg = strtok(arg, " ")) == 0))
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
        if (arg == nullptr)
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

        if (arg == nullptr)
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
    if (token == nullptr)
    {
        LOGGER_ERR("Failed to parse configuration line: no command");
        return false;
    }
    errno = 0;
    cmd = (enum bk_cmd)strtol(token, nullptr, 10);
    if (errno != 0)
    {
        LOGGER_ERR("Failed to parse configuration line: command should be a decimal integer [bk_cmd=%s]", token);
        return false;
    }

    // Retrieve block identifier
    token = strtok(nullptr, " ");
    if (token == nullptr)
    {
        LOGGER_ERR("Failed to parse configuration line: no block identifier");
        return false;
    }
    errno = 0;
    id = (enum bk_cmd)strtol(token, nullptr, 10);
    if (errno != 0)
    {
        LOGGER_ERR("Failed to parse configuration line: block identifier should be a decimal integer [bk_id=%s]", token);
        return false;
    }

    // Retrieve argument (optional)
    arg = strtok(nullptr, "\0");

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
    if (file == nullptr)
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
        line = nullptr;
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
    size_t w;

    LOGGER_DEBUG("Getting blocks information")

    w = 0;
    auto i = bk_map_.begin();
    const auto &e = bk_map_.end();
    while (i != e)
    {
        int ret;

        ret = snprintf(buf + w, len - w, "%u %s %d;", i->first, i->second->type_.c_str(), i->second->state_);
        if (ret < 0)
        {
            LOGGER_ERR("snprintf failed");
            break;
        }
        else
        {
            w += static_cast<size_t>(ret);
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
