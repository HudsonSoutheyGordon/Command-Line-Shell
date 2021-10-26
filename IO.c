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
#include "IO.h"

int inputFile = -1;
int outputFile = -1;

/*
* Closes the input and output files if they exist.
* Registered after forking to the child's atexit() to ensure we always close open files.
*/
void closeFiles(void) {
    if (inputFile > -1) {
        close(inputFile);
    }
    if (outputFile > -1) {
        close(outputFile);
    }

}

void redirectInput(ParsedInput* pi) {
    if (pi->input == NULL) {
        return;
    }

    int targetFD = open(pi->input, O_RDONLY, 0740);

    if (targetFD == -1) {
        utilPrintf("Could not open the designated output file!\n");
        exit(1);
    }

    // Use dup2 to point FD 1, i.e., standard output to targetFD
    int result = dup2(targetFD, 1);
    if (result == -1) {
        perror("dup2");
        exit(2);
    }

    inputFile = targetFD;
}

/*
* Redirects the stdout to the parsed input.
* Sets the decodedExit flag on a fail.
* Modified from canvas exploration: process and I/O
*/
void redirectOutput(ParsedInput* pi) {
    if (pi->output == NULL) {
        return;
    }

    int targetFD = open(pi->input, O_WRONLY | O_CREAT | O_TRUNC, 0740);

    if (targetFD == -1) {
        utilPrintf("Could not open the designated output file!\n");
        exit(1);
    }

    // Use dup2 to point FD 1, i.e., standard output to targetFD
    int result = dup2(targetFD, 1);
    if (result == -1) {
        perror("dup2");
        exit(2);
    }

    outputFile = targetFD;
}