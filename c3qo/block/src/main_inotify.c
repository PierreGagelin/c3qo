/**
 * @brief Main file of the engine
 */

#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>

#include "../../../block/block.h"

int main (int argc, char **argv)
{
        /* unused parameter */
        (void) argc;
        (void) argv;

        void *          libblock;
        struct block_if *block;
        char *          error;

        libblock = dlopen("libblock.so", RTLD_LAZY);
        if (libblock == NULL)
        {
                LOGGER_ERR("%s\n", dlerror())
                exit(EXIT_FAILURE);
        }

        /* Retrieve, initialize and start the block */
        dlerror(); /* clear previous errors */
        block = (struct block_if *) dlsym(libblock, "inotify_sb_entry");
        error = dlerror();
        if (error != NULL)
        {
                LOGGER_ERR("%s\n", dlerror())
                exit(EXIT_FAILURE);
        }
        block->ctrl(BLOCK_INIT, NULL);
        block->ctrl(BLOCK_START, NULL);

        getchar();

        block->ctrl(BLOCK_STOP, NULL);

        exit(EXIT_SUCCESS);
}



