/**
 * @brief Main file of the engine
 */


#include <dlfcn.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../../../block/block.h"

int main (int argc, char ** argv)
{
        /* unused parameter */
        (void) argv;

        void *            libblock;
        struct block_if * block;
        const char *      block_name;
        char *            error;

        libblock = dlopen("libblock.so", RTLD_LAZY);
        if (libblock == NULL)
        {
                fprintf(stderr, "%s\n", dlerror());
                exit(EXIT_FAILURE);
        }

        /* Either as a server or a client */
        if (argc == 1)
        {
                block_name = "server_us_asnb_entry";
        }
        else
        {
                block_name = "client_us_asnb_entry";
        }

        /* Retrieve, initialize and start the block */
        dlerror(); /* clear previous errors */
        block = (struct block_if *) dlsym(libblock, block_name);
        error = dlerror();
        if (error != NULL)
        {
                fprintf(stderr, "%s\n", dlerror());
                exit(EXIT_FAILURE);
        }
        block->ctrl(NULL, NULL, NULL, BLOCK_INIT, NULL);
        block->ctrl(NULL, NULL, NULL, BLOCK_START, NULL);

        /* sleep, it will be woken up by a signal */
        while (true)
        {
                pause();
        }

        exit(EXIT_SUCCESS);
}



