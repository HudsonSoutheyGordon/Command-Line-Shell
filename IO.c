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

/*
* Redirects the input (if necessary) to the proper source.
* BG commands without redirection go to /dev/null
*/
void redirectInput(ParsedInput* pi) {
    int targetFD;
    // If there was no redirection for a FG process, simply return.
    if (pi->input == NULL && !pi->isBG) {
        return;
    }
    // If there was no redirection for a BG process, set input to /dev/null
    else if (pi->input == NULL && pi->isBG) {
        targetFD = open("/dev/null", O_RDONLY);
    }
    else {
        targetFD = open(pi->input, O_RDONLY, 0640);
    }

    if (targetFD == -1) {
        printf("Could not open input file: %s", pi->input);
        fflush(stdout);
        exit(1);
    }

    // Use dup2 to point FD 0, i.e., standard input to targetFD
    int result = dup2(targetFD, 0);
    if (result == -1) {
        perror("dup2");
        exit(2);
    }

    inputFile = targetFD;
}

/*
* Redirects the stdout to the parsed input.
* Modified from canvas exploration: process and I/O.
* BG commands without redirection go to /dev/null
*/
void redirectOutput(ParsedInput* pi) {
    int targetFD;
    // If there was no redirection for a FG process, simply return.
    if (pi->output == NULL && !pi->isBG) {
        return;
    }
    // If there was no redirection for a BG process, set output to /dev/null
    else if (pi->output == NULL && pi->isBG) {
        targetFD = open("/dev/null", O_WRONLY, 0640);
    }
    else {
        targetFD = open(pi->output, O_WRONLY | O_CREAT | O_TRUNC, 0640);
    }

    if (targetFD == -1) {
        printf("Could not open output file: %s", pi->output);
        fflush(stdout);
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