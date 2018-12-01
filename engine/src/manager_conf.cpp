

#define LOGGER_TAG "[engine.conf]"

// Project headers
#include "engine/manager.hpp"

// Generated protobuf command
#include "conf.pb-c.h"

//
// @brief Execute a block command
//
// @return True on success
//
bool manager::conf_exec_cmd(enum bk_cmd cmd, int id, char *arg)
{
    LOGGER_DEBUG("Execute block command [bk_cmd=%s ; bk_id=%d ; bk_arg=%s]", bk_cmd_to_string(cmd), id, arg);

    if (id == 0)
    {
        LOGGER_ERR("Failed to execute block command: id 0 is forbidden");
        return false;
    }

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
    case CMD_START:
    {
        return block_start(id);
    }
    case CMD_STOP:
    {
        return block_stop(id);
    }
    case CMD_DEL:
    {
        return block_del(id);
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
            LOGGER_ERR("Failed to bind block: configuration entry required as third argument");
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
bool manager::conf_parse_line(char *line)
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

    return conf_exec_cmd(cmd, id, arg);
}

//
// @brief Parse a configuration file to retrieve block configuration
//
// @param filename : Name of the configuration file
//
bool manager::conf_parse(const char *filename)
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
size_t manager::conf_get(char *buf, size_t len)
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

//
// @brief Parse and execute a protobuf command
//
bool manager::conf_parse_pb_cmd(const uint8_t *pb_cmd, size_t size)
{
    Command *cmd;
    bool ret;

    cmd = command__unpack(nullptr, size, pb_cmd);
    if (cmd == nullptr)
    {
        LOGGER_ERR("Failed to unpack protobuf command: unknown reason [size=%zu]", size);
        return false;
    }

    switch (cmd->type_case)
    {
    case COMMAND__TYPE_ADD:
        ret = conf_exec_cmd(CMD_ADD, cmd->add->id, cmd->add->type);
        break;
    case COMMAND__TYPE_START:
        ret = conf_exec_cmd(CMD_START, cmd->start->id, nullptr);
        break;
    case COMMAND__TYPE_STOP:
        ret = conf_exec_cmd(CMD_STOP, cmd->stop->id, nullptr);
        break;
    case COMMAND__TYPE_DEL:
        ret = conf_exec_cmd(CMD_DEL, cmd->del->id, nullptr);
        break;
    case COMMAND__TYPE_CONF:
        ret = conf_exec_cmd(CMD_CONF, cmd->conf->id, cmd->conf->conf);
        break;
    case COMMAND__TYPE_BIND:
        ret = block_bind(cmd->bind->id, cmd->bind->port, cmd->bind->dest);
        break;
    case COMMAND__TYPE__NOT_SET:
    default:
        LOGGER_ERR("Failed to execute protobuf command: unknown command type [type=%d]", cmd->type_case);
        goto err;
    }

    command__free_unpacked(cmd, nullptr);
    return ret;
err:
    command__free_unpacked(cmd, nullptr);
    return false;
}
