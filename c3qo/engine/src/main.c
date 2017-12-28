

#include <stdlib.h>

#include "manager.h"


int main(char argc, char ** argv)
{
        (void) argc;
        (void) argv;

        /* Parse configuration file */
        manager_parse_conf("/tmp/config.txt");

        return 0;
}


