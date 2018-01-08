#ifndef C3QO_MANAGER_H
#define C3QO_MANAGER_H

#include <stdbool.h>


/**
 * @brief : Parse a file to retrieve block configuration
 *          Each line is a configuration entry using this syntax :
 *            - <bk_cmd> <bk_id> <cmd_arg>\n
 *            - <bk_cmd> : Command to process
 *              - BK_ADD
 *              - BK_INIT
 *              - BK_CONFIGURE
 *              - BK_BIND
 *              - BK_START
 *              - BK_STOP
 *            - <bk_id> : Number that identifies the block (the one given in BK_ADD)
 *            - <cmd_arg> : Arguments specific to the command (see each command in specific)
 */
bool   manager_conf_parse(const char *filename);
size_t manager_conf_get(char *buf, size_t len);

void manager_block_clean();


#endif


