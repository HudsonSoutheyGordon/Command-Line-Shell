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

    initChildren();
    registerSigHandlers();

    char* inputtedString;
    inputtedString = calloc(maxInputLength, sizeof(char));

    int* hasBGChild = malloc(sizeof(int));
    *hasBGChild = 0;
    
    // The first int is the value, the second is a bool indicating if it terminated normally.
    // 0 = normal termination
    // 1 = abnormal termination
    // We alloc the array and then initialize the values to 0, 0
    // These are used for the entirety of the program and thus do not get free()'d.
    int** fgExitStatus = (int**)calloc(2, sizeof(int));
    fgExitStatus[0] = malloc(sizeof(int));
    *(fgExitStatus[0]) = 0;
    fgExitStatus[1] = malloc(sizeof(int));

    while (true) {

        if (*hasBGChild > 0) {
            bgCheck(hasBGChild);
        }


        utilPrintf(": ");

        fgets(inputtedString, maxInputLength, stdin);
        // to removie the trailing \n - Citation: https://stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input
        if (!inputtedString[0] == '\0') {
            inputtedString[strcspn(inputtedString, "\n")] = 0;
        }

        if (inputtedString[0] == '#' || inputtedString[0] == '\0') {
            // Do nothing and repeat the loop
        }
        else {
            //  NOT ARGUMENTS WILL NOT HAVE SPACES, thus we do not need to worry about echo $$ somthing else 
            char* updatedStr = pidExpansion(inputtedString);
            ParsedInput* pi = parseInput(updatedStr);
            // Determine if the command was one of our built ins.
            char* exitString = "exit";
            if (strcmp(pi->command, exitString) == 0) {
                shellExit();
            } else if(strcmp(pi->command, "status") == 0) {
                shellStatus(fgExitStatus);
            }
            else if (strcmp(pi->command, "cd") == 0) {
                shellCD(pi->args[1]);
            }
            else {
                externalFunc(pi, hasBGChild, fgExitStatus);
            }

            // We have done everything with the command, let's free up its struct
            freeParsedInput(pi);
            //free(updatedStr);
        }



    }

    return EXIT_SUCCESS;

}
