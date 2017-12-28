

#include <stdio.h>  /* fopen, fgets, sscanf */
#include <stdlib.h> /* malloc, free, strtoul */
#include <unistd.h> /* sysconf */
#include <stdint.h> /* fixed-size data types */

#include "c3qo/block.h"
#include "c3qo/logger.h"

#include "manager.h"




/* Each block shall be linked */
extern struct block_if hello_entry;
extern struct block_if goodbye_entry;

/* List of pointers to block interfaces */
const uint16_t         BLOCK_ID_MAX = UINT16_MAX - 1;
static struct block_if *block_list[UINT16_MAX];

/* Generic command to manage a block */
struct manager_cmd 
{
        enum block_cmd cmd;
        uint16_t       id;
        char           arg[4096];

};

/* Using the same instance to minimize stack pression */
struct manager_cmd cmd;


/**
 * @brief : Create a block interface
 *
 * @param block_id   : Identifier in the block list
 * @param block_type : Type of interface to implement
 */
static void manager_block_add(uint16_t block_id, enum block_type type)
{
        block_list[block_id] = (struct block_if *)
                               malloc(sizeof(struct block_if));
        if (block_list[block_id] == NULL)
        {
                LOGGER_CRIT("Out of memory!");
                return;
        }

        switch (type)
        {
        case BLOCK_HELLO:
        {
                *block_list[block_id] = hello_entry;
                break;
        }
        case BLOCK_GOODBYE:
        {
                *block_list[block_id] = goodbye_entry;
                break;
        }
        default:
        {
                LOGGER_WARNING("Block type does not exist");
                break;
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
        case BLOCK_ADD:
        {
                unsigned long int block_type;
                
                if (block_list[cmd.id] != NULL)
                {
                        LOGGER_WARNING("Cannot add an existing block");
                        break;
                }

                block_type = strtoul(cmd.arg, NULL, 10);
                if (block_type > BLOCK_TYPE_MAX)
                {
                        LOGGER_WARNING("Block type does not exist");
                }

                manager_block_add(cmd.id, (enum block_type) block_type);

                break;
        }
        case BLOCK_INIT:
        case BLOCK_CONFIGURE:
        case BLOCK_BIND:
        case BLOCK_START:
        case BLOCK_STOP:
        {
                if (block_list[cmd.id] == NULL);
                {
                        LOGGER_ERR("Can't find block");
                        break;
                }

                block_list[cmd.id]->ctrl(cmd.cmd, cmd.arg);
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

        nb_arg = fscanf(file,
                     "%u %u %4095s\n",
                     &u_cmd,
                     &u_id,
                     cmd.arg);
        if (nb_arg != 3)
        {
                LOGGER_ERR("Corrupted configuration entry");
                return -2;
        }

        /* Check values (shouldn't stop the configuration) */
        if (u_cmd > BLOCK_CMD_MAX)
        {
                LOGGER_WARNING("block_cmd=%u does not exist", u_cmd);
                return -1;
        }
        if (u_id > BLOCK_ID_MAX)
        {
                LOGGER_WARNING("block_id=%u can't fit on 16 bits", u_id);
                return -1;
        }

        cmd.cmd = (enum block_cmd) u_cmd;
        cmd.id  = (uint16_t) u_id;

        return 1;
}


/**
 * @brief Clean all blocks
 */
void manager_clean()
{
        uint16_t id;

        for (id = 0; id < UINT16_MAX; id++)
        {
                if (block_list[id] == NULL)
                {
                        continue;
                }

                block_list[id]->ctrl(BLOCK_STOP, NULL);
                free(block_list[id]);
                block_list[id] = NULL;
        }
}


/**
 * @brief : Parse a configuration file to retrieve block configuration
 *
 * @param filename : configuration file name
 *
 * @note : See header file for description
 */
bool manager_parse_conf(const char *filename)
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


