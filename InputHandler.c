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
#include "Util.h"

#define maxArgs 512

/*
* Helper function.
*
* Takes the current token, and determines what kind of input it is.
* Returns an int specifying as such.
*
* Will return an int corresponding to:
*
* NULL     : 0 -> Will happen when we reach the end of the input
* <        : 1
* >        : 2
* &        : 3
* argument : 4
*
* Do not use for the initial command parse!
*/
int determineToken(char* token) {

    char* cmpStr1 = "<";
    char* cmpStr2 = ">";
    char* cmpStr3 = "&";

    if (token == NULL) {
        return 0;
    }
    else if (strcmp(token, cmpStr1) == 0) {
        return 1;
    }
    else if (strcmp(token, cmpStr2) == 0) {
        return 2;
    }
    else if (strcmp(token, cmpStr3) == 0) {
        return 3;
    }
    else {
        return 4;
    }
}

/*
* Will parse the input of the given command and other params.
* Will create, populate, and return the parsed data in the form of a pointer to a ParsedInput struct.
*/
ParsedInput* parseInput(char* inputStr) {
	ParsedInput* currInput = malloc(sizeof(ParsedInput));
    currInput->args = (char**)calloc(maxArgs, sizeof(char*));  // Allocate memory for an array of str pointers
    int argsIndex = 0;

    currInput->isBG = false; // Default to false

	// For use with strtok_r
	char* saveptr;

    // The first token is the command. We parse, allocate, and copy it.
    char* token = strtok_r(inputStr, " ", &saveptr);
    currInput->command = calloc(strlen(token) + 1, sizeof(char));
    strcpy(currInput->command, token);

    /*
    * Now we are in a position, where the next token could be any of the following:
    * args for the command,
    * <,
    * >,
    * &,
    * Or nothing.
    * We use the helper function, determineToken(), to decide which of these options it is.
    */
    int tokenType;
    do {
        token = strtok_r(NULL, " ", &saveptr);
        tokenType = determineToken(token);

        // DEBUG: I BELIEVE IT TAKES arg1 and thinks it is < in determineToken()

        switch(tokenType) {
            case 0:     // NULL : so break and the loop will break
                break;

            case 1:     // < : thus the next token is a location for input
                // Parse the next word because we don't actually care about the <
                token = strtok_r(NULL, " ", &saveptr);
                currInput->input = calloc(strlen(token) + 1, sizeof(char));
                strcpy(currInput->input, token);
                break;

            case 2:     // > : thus the next token is a location for output
                // Parse the next word because we don't actually care about the >
                token = strtok_r(NULL, " ", &saveptr);
                currInput->output = calloc(strlen(token) + 1, sizeof(char));
                strcpy(currInput->output, token);
                break;

            case 3:     // & : thus this command should run in the background
                currInput->isBG = true;
                break;

            case 4:     // This is an argument after the command
                currInput->args[argsIndex] = calloc(strlen(token) + 1, sizeof(char));   // Allocate for the pointer to the argument string
                strcpy(currInput->args[argsIndex], token);
                argsIndex++;

                if (argsIndex > maxArgs) {
                    perror("Too many arguments given!");
                    exit(1);
                }
                break;

            default:
                perror("Error parsing the input");
                break;
        }

    } while (tokenType != 0);


    return currInput;
}

