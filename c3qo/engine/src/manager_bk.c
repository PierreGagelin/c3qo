

#include <stdio.h>  /* fopen, fgets, sscanf */
#include <stdlib.h> /* malloc, free, strtoul */
#include <unistd.h> /* sysconf */
#include <stdint.h> /* fixed-size data types */

#include "c3qo/block.h"
#include "c3qo/logger.h"
#include "c3qo/manager_bk.h"


/* Each block shall be linked */
extern struct bk_if hello_entry;
extern struct bk_if goodbye_entry;
extern struct bk_if inotify_sb_entry;
extern struct bk_if client_us_nb_entry;
extern struct bk_if server_us_nb_entry;

/* List of pointers to block interfaces */
struct bk_info
{
        struct bk_if  bk;
        enum bk_type  type;
        enum bk_state state;
};
struct bk_info *bk_list[UINT16_MAX];
uint16_t       bk_list_count;

/* Generic command to manage a block */
struct manager_cmd 
{
        enum bk_cmd cmd;
        uint16_t    id;
        char        arg[4096];

};

/* Using the same instance to minimize stack pression */
struct manager_cmd cmd;


/**
 * @brief : Create a block interface
 *
 * @param id   : Identifier in the block list
 * @param type : Type of interface to implement
 */
static void manager_block_add(uint16_t id, enum bk_type type)
{
        if (type > BK_TYPE_MAX)
        {
                LOGGER_WARNING("Block type does not exist");
                return;
        }

        bk_list[id] = (struct bk_info *) malloc(sizeof(*bk_list[id]));

        if (bk_list[id] == NULL)
        {
                LOGGER_CRIT("Out of memory!");
                return;
        }

        LOGGER_DEBUG("Add block bk_id=%u, bk_type=%d", id, type);
        bk_list_count++;
        bk_list[id]->type  = type;
        bk_list[id]->state = BK_STOPPED;

        switch (type)
        {
        case BK_HELLO:
        {
                bk_list[id]->bk = hello_entry;
                break;
        }
        case BK_GOODBYE:
        {
                bk_list[id]->bk = goodbye_entry;
                break;
        }
        case BK_INOTIFY_SB:
        {
                bk_list[id]->bk = inotify_sb_entry;
                break;
        }
        case BK_CLIENT_US_ASNB:
        {
                bk_list[id]->bk = client_us_nb_entry;
                break;
        }
        case BK_SERVER_US_ASNB:
        {
                bk_list[id]->bk = server_us_nb_entry;
                break;
        }
        default:
        {
                /* This case mean all values in enum bk_type aren't used */
                LOGGER_CRIT("Block type enumerate incomplete");
                free(bk_list[id]);
                bk_list[id] = NULL;
                bk_list_count--;
                return;
        }
        }
}


/**
 * @brief : Execute the global manager command
 *          Some of them are directly executed by a block
 */
static void manager_exec_cmd()
{
        switch (cmd.cmd)
        {
        case BK_ADD:
        {
                unsigned long int bk_type;
                
                if (bk_list[cmd.id] != NULL)
                {
                        LOGGER_WARNING("Cannot add an existing block");
                        break;
                }

                bk_type = strtoul(cmd.arg, NULL, 10);

                manager_block_add(cmd.id, (enum bk_type) bk_type);

                break;
        }
        case BK_INIT:
        case BK_CONFIGURE:
        case BK_BIND:
        case BK_START:
        case BK_STOP:
        {
                if (bk_list[cmd.id] == NULL);
                {
                        LOGGER_ERR("Can't find block");
                        break;
                }

                bk_list[cmd.id]->bk.ctrl(cmd.cmd, cmd.arg);
                break;
        }
        default:
        {
                /* Ignore this entry */
                LOGGER_WARNING("Unknown block command %u", cmd.cmd);
        }
        }
}


/**
 * @brief : Get a configuration line and fill the global manager command
 *
 * @param file : configuration file
 *
 * @return : Several values
 *             - -2 on bad parsing
 *             - -1 on bad values
 *             -  0 if there are no more lines
 *             -  1 on success
 */
static int manager_conf_parse_line(FILE *file)
{
        int          nb_arg;
        unsigned int u_cmd; /* uint16_t is too short for %u */
        unsigned int u_id;  /* uint16_t is too short for %u */

        if (feof(file) != 0)
        {
                LOGGER_DEBUG("Finished to read configuration file");
                return 0;
        }

        nb_arg = fscanf(file, "%u %u %4095s\n", &u_cmd, &u_id, cmd.arg);
        if (nb_arg != 3)
        {
                LOGGER_ERR("Corrupted configuration entry");
                return -2;
        }

        /* Check values (shouldn't stop the configuration) */
        if (u_cmd > BK_CMD_MAX)
        {
                LOGGER_WARNING("bk_cmd=%u does not exist", u_cmd);
                return -1;
        }
        if (u_id >= UINT16_MAX)
        {
                LOGGER_WARNING("bk_id=%u can't fit on 16 bits", u_id);
                return -1;
        }

        cmd.cmd = (enum bk_cmd) u_cmd;
        cmd.id  = (uint16_t) u_id;

        return 1;
}


/**
 * @brief : Clean all blocks
 */
void manager_block_clean()
{
        uint16_t id;

        for (id = 0; id < UINT16_MAX; id++)
        {
                if (bk_list[id] == NULL)
                {
                        continue;
                }

                bk_list[id]->bk.ctrl(BK_STOP, NULL);
                free(bk_list[id]);
                bk_list[id] = NULL;
                bk_list_count--;

                /* Stop if all blocks are already deallocated */
                if (bk_list_count == 0)
                {
                        break;
                }
        }
}


/**
 * @brief : Fill a string representing existing blocks
 *          Format for each entry follows :
 *            - <bk_id> <bk_type> <bk_state>;
 *
 * @param buf : string to fill
 * @param len : maximum length to write
 *
 * @return : actual length written
 */
size_t manager_conf_get(char *buf, size_t len)
{
        size_t   w = 0;
        uint16_t i;
        uint16_t c = 0; /* Count of block information dumped */

        LOGGER_DEBUG("Getting blocks information")

        for (i = 0; i < UINT16_MAX; i++)
        {
                int ret;

                if (bk_list[i] == NULL)
                {
                        continue;
                }

                c++;
                ret = snprintf(buf + w, len - w, "%u %d %d;", i, bk_list[i]->type, bk_list[i]->state);
                if (ret < 0)
                {
                        LOGGER_ERR("snprintf failed");
                        break;
                }
                else
                {
                        w += (size_t) ret;
                }

                /* Stop if all blocks are already dumped */
                if (c == bk_list_count)
                {
                        break;
                }

                /* Stop if there's no more room in buf */
                if (w >= len)
                {
                        LOGGER_WARNING("Not enough space to dump blocks information");
                        w = len;
                        break;
                }
        }

        return w;
}


/**
 * @brief : Parse a configuration file to retrieve block configuration
 *
 * @param filename : configuration file name
 *
 * @note : See header file for description
 */
bool manager_conf_parse(const char *filename)
{
        FILE *file;
        bool ret = true;

        LOGGER_INFO("Reading configuration from '%s'", filename);

        file = fopen(filename, "r");
        if (file == NULL)
        {
                LOGGER_ERR("Couldn't open '%s'", filename);
                return false;
        }

        /* Read one line at a time */
        while (true)
        {
                int rc;

                rc = manager_conf_parse_line(file);
                if (rc == -2)
                {
                        /* Shouldn't read anymore */
                        ret = false;
                        break;
                }
                else if (rc == -1)
                {
                        /* Should not use values */
                        continue;
                }
                else if (rc == 0)
                {
                        break;
                }

                manager_exec_cmd();
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


