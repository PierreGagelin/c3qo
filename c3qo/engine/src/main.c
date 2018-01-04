

#include <stdlib.h>
#include <unistd.h> /* getopt */

#include "c3qo/logger.h"
#include "c3qo/manager.h"


extern char *optarg;


int main(char argc, char ** argv)
{
        bool       conf;
        int        opt;
        const char *filename = "/tmp/config.txt";
        int        ret = 0;

        LOGGER_OPEN();

        while ((opt = getopt(argc, argv, "hf:")) != -1)
        {
                switch (opt)
                {
                case 'h':
                {
                        LOGGER_DEBUG("lol, help is for the weaks");
                        break;
                }
                case 'f':
                {
                        LOGGER_DEBUG("CLI file to load configuration : %s", optarg);
                        filename = optarg;
                        break;
                }
                default:
                {
                        LOGGER_ERR("CLI argument error");
                        break;
                }
                }
        }

        /* Parse configuration file */
        conf = manager_conf_parse(filename);
        if (conf == false)
        {
                ret = -1;
        }

        LOGGER_CLOSE();

        return ret;
}


