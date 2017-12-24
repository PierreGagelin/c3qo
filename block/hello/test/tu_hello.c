/**
 * @brief Test file for the hello block
 */


#include <stdlib.h>

#include "block/hello.h"


int main (int argc, char **argv)
{
        (void) argc;
        (void) argv;

        hello_entry.ctrl(BLOCK_INIT, NULL);
        hello_entry.ctrl(BLOCK_START, NULL);

        exit(EXIT_SUCCESS);
}



