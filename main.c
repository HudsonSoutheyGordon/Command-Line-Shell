// If you are not compiling with the gcc option --std=gnu99, then
// uncomment the following line or you might get a compiler warning
//#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include "InputHandler.h"



int main(int argc, char* argv[])
{
    printf("\n\n ############## START OF PROGRAM ##############\n\n");

    char* myString;

    parseInput(myString);

    return EXIT_SUCCESS;

    // Arbitrary comment
}
