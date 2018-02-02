

#include <stdlib.h>
#include <unistd.h> /* getopt */

#include "c3qo/logger.hpp"
#include "c3qo/manager_bk.hpp"


extern char *optarg;


int main(int argc, char ** argv)
{
        bool       conf;
        int        opt;
        const char *filename = "/tmp/config.txt";
        int        ret = 0;

        LOGGER_OPEN();
        logger_set_level(LOGGER_LEVEL_MAX);

        while ((opt = getopt(argc, argv, "hf:l:")) != -1)
        {
                switch (opt)
                {
                case 'h':
                {
                        LOGGER_DEBUG("CLI help : lol, help is for the weaks");
                        break;
                }
                case 'f':
                {
                        LOGGER_DEBUG("CLI file to load configuration : %s", optarg);
                        filename = optarg;
                        break;
                }
                case 'l':
                {
                        unsigned long int level;

                        LOGGER_DEBUG("CLI setting log level to %s", optarg);

                        level = strtoul(optarg, NULL, 10);
                        logger_set_level((enum logger_level) level);

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


