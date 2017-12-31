

#include <stdlib.h>

#include "c3qo/logger.h"
#include "c3qo/manager.h"


int main(char argc, char ** argv)
{
        bool conf;
        int  ret = 0;

        (void) argc;
        (void) argv;

        LOGGER_OPEN();

        /* Parse configuration file */
        conf = manager_conf_parse("/tmp/config.txt");
        if (conf == false)
        {
                ret = -1;
        }

        LOGGER_CLOSE();

        return ret;
}


