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
    if (forkCount > 50) {
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
Debug function to get current working directory.
Implementing this prior to having other solutions set up.
*/
void debugPWD(void) {
    char* buf = getcwd(NULL, 0);
    utilPrintf(buf);
    utilPrintf("\n");
    free(buf);
}

/*
* Frees the members of the ParsedInput struct
* in the reverse order that they were alloc'd.
* Then free the struct itself.
*/
void freeParsedInput(ParsedInput* pi) {

    free(pi->output);
    free(pi->input);
    // Iterate through our array of args and free them all
    for (int i = 0; i < pi->argCount; i++) {
        free(pi->args[i]);
    }
    free(pi->args);
    free(pi->command);

    free(pi);
}

/*
* Tokenize the PATH variable.
* Returns a char** which points to all the paths that were parsed
*/
char** tokenizePATH(int* pathsLength) {
    char* homeEnvVar = getenv("PATH");

    // Start with an array of ~30 pointers. Will realloc if we exceed
    int callocSize = 30;
    char** paths = (char**)calloc(callocSize, sizeof(char*));

    // For use with strtok_r
    char* saveptr;
    char* token = strtok_r(homeEnvVar, ":", &saveptr);

    // Edge case of an empty path variable
    if (token == NULL) {
        perror("Your PATH variable is empty. Thus there is no such program.\n");
        fflush(stderr);
        // TO DO: Raise error
        return NULL;
    }

    int pathsIndex = 0;
    while (token != NULL) {

        // If we've reached the end of our allocated space. We need to resize.
        if (pathsIndex == (callocSize - 1)) {
            callocSize += 10;
            paths = (char**)realloc(paths, callocSize * sizeof(char*));
        }

        // Place the parsed path string into our array of strings
        paths[pathsIndex] = token;
        pathsIndex++;

        token = strtok_r(NULL, ":", &saveptr);
    }
    // We dereference our int pointer to pass back the length of our array of paths.
    *pathsLength = pathsIndex;
    return paths;
}




