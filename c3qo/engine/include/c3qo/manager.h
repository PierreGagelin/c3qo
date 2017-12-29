

#include <stdbool.h>


/**
 * @brief : Parse a file to retrieve block configuration
 *          Each line is a configuration entry using this syntax :
 *            - <block_cmd> <block_id> <cmd_arg>\n
 *            - <block_cmd> : Command to process
 *              - BLOCK_ADD
 *              - BLOCK_INIT
 *              - BLOCK_CONFIGURE
 *              - BLOCK_BIND
 *              - BLOCK_START
 *              - BLOCK_STOP
 *            - <block_id> : Number that identifies the block (the one given in BLOCK_ADD)
 *            - <cmd_arg> : Arguments specific to the command (see each command in specific)
 */
bool manager_parse_conf(const char *filename);


