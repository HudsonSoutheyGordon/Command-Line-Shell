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
#include "BuiltIns.h"
#include "Util.h"
#include "ChildMan.h"
#include "SigMan.h"

/*
* Wrapper for printf which also always fflushes stdout
*/
void utilPrintf(char* str) {
    printf(str);
    fflush(stdout);
}
/*
* Wrapper function for fork().
* If we have forked 25 times, abort to avoid a forkBomb
* Resets whenever a command is inputted which indicates normal functioning
*/
int forkCount = 0;
pid_t forkSafe(void) {
    forkCount++;
    if (forkCount > 25) {
        perror("ForkBomb");
        fflush(stderr);
        abort();
    }

    pid_t retPid = fork();
    return retPid;
}

// Resets the above forkCount
void resetForkBombCounter(void) {
    forkCount = 0;
}

/*
* Frees the members of the ParsedInput struct
* in the reverse order that they were alloc'd.
* Then free the struct itself.
*/
void freeParsedInput(ParsedInput* pi) {

    if (pi->output != NULL) {
        free(pi->output);
        pi->output = NULL;
    }
    if (pi->input != NULL) {
        free(pi->input);
        pi->input = NULL;
    }
    // Iterate through our array of args and free them all
    for (int i = 0; i < pi->argCount; i++) {
        free(pi->args[i]);
    }

    free(pi->args);
    pi->args = NULL;
    free(pi->command);
    pi->command = NULL;

    free(pi);
}




