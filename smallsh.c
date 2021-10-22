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

#define maxInputLength 2048

void printBackInput(ParsedInput* pi) {
    printf("---- The Input was: ----\n");
    fflush(stdout);
    printf("Command: %s\n", pi->command);
    fflush(stdout);
    char* currentArg = pi->args[0];
    int argIndex = 0;
    while(currentArg != NULL) {
        printf("Argument %d: %s\n", argIndex, currentArg);
        fflush(stdout);
        argIndex++;
        currentArg = pi->args[argIndex];
    }
    printf("Input location: %s\n", pi->input);
    fflush(stdout);
    printf("Output location: %s\n", pi->output);
    fflush(stdout);
    if (pi->isBG) {
        utilPrintf("Run in BG: true\n");
    }
    else {
        utilPrintf("Run in BG: false\n");
    }
}

int main(int argc, char* argv[])
{
    utilPrintf("\n\n ############## START OF PROGRAM ##############\n\n");

    char* inputtedString;
    inputtedString = calloc(maxInputLength, sizeof(char));

    while (true) {
        utilPrintf(": ");

        fgets(inputtedString, maxInputLength, stdin);
        // fgets leaves a trailing \n - Citation: https://stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input
        if (!inputtedString[0] == '\0') {
            inputtedString[strcspn(inputtedString, "\n")] = 0;
        }
        
        if (inputtedString[0] == '#' || inputtedString[0] == '\0') {
            // Do nothing and repeat the loop
        }
        else {
            //printBackInput(parseInput(inputtedString));
            ParsedInput* pi = parseInput(inputtedString);
            
            // Determine if the command was one of our built ins.
            char* exitString = "exit";
            if (strcmp(pi->command, exitString) == 0) {
                shellExit();
            } else if(strcmp(pi->command, "status") == 0) {
                shellStatus();
            }
            else if (strcmp(pi->command, "cd") == 0) {
                shellCD(pi->args[0]);
            }
            else {
                utilPrintf("TODO: Set up with exec");
                // Run using exec()
            }

        }

    }

    return EXIT_SUCCESS;

    // Arbitrary comment
}
