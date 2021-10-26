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
#include "ChildMan.h"
#include "SigMan.h"

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
* Takes the original inputted string and expands any instance of $$ into the shell's pid.
* Returns a pointer to the string with $$ expanded.
*/
char* pidExpansion(char* inputStr) {

    //char* myString = calloc(strlen(inputStr) + 1, sizeof(char));
    //strcpy(myString, inputStr);
    char* myString = strdup(inputStr);
    char* myStringStartPtr = myString;

    int pid = getpid();
    char* myPid = calloc(10, sizeof(char));
    sprintf(myPid, "%d", pid);

    // Check if we even need to expand
    if (strstr(myString, "$$") == NULL) {
        return inputStr;
    }

    // Section Initializing vars
    char* endAddress = myString + strlen(myString);
    int subStringIndex = 0;
    int subStrCount = 4;
    char** substrings = (char**)malloc(subStrCount * sizeof(char*));
    bool shouldParse = true;
    bool hasTail = false;
    int count = 0;
    char* tailStr = NULL;
    char* updatedStr;

    while (shouldParse) {
        int subStringLen = strstr(myString, "$$") - myString;

        // Allocate for the substring we found based on the subStringlength
        if (subStringIndex == subStrCount - 1) {
            subStrCount *= 2;
            substrings = (char**)realloc(substrings, subStrCount * (sizeof(char*)));
        }

        substrings[subStringIndex] = calloc(subStringLen, sizeof(char));
        count++;
        // Add it to the array of substrings
        strncpy(substrings[subStringIndex], myString, subStringLen);

        // Now do the same for pid
        subStringIndex++;

        if (subStringIndex == subStrCount - 1) {
            subStrCount *= 2;
            substrings = (char**)realloc(substrings, subStrCount * (sizeof(char*)));
        }

        substrings[subStringIndex] = calloc(strlen(myPid) + 1, sizeof(char));
        strcpy(substrings[subStringIndex], myPid);

        subStringIndex++;
        // Move the pointer forward
        myString += subStringLen + 2;

        // Passed the ending address
        if (myString >= endAddress) {
            //printf("PassedEnding\n");
            shouldParse = false;
        }
        // No more $$ in string
        else if (strstr(myString, "$$") == NULL) {

            if (subStringIndex == subStrCount - 1) {
                subStrCount *= 2;
                substrings = (char**)realloc(substrings, subStrCount * (sizeof(char*)));
            }
            substrings[subStringIndex] = calloc(strlen(myString) + 1, sizeof(char));
            strcpy(substrings[subStringIndex], myString);
            //printf("We are done here. Got a null\n");
            shouldParse = false;
            hasTail = true;
            subStringIndex++;
        }

    }

    if (!hasTail) {
        updatedStr = (char*)malloc(strlen(myStringStartPtr)*sizeof(char) + (count * strlen(myPid) * sizeof(char)) - (count * 2 * sizeof(char)));
    }
    else {
        updatedStr = (char*)malloc(strlen(myStringStartPtr) * sizeof(char) + (count * strlen(myPid) * sizeof(char)) + (strlen(myStringStartPtr) * sizeof(char)) - (count * 2 * sizeof(char)));
    }

    int i = 0;
    while (i < subStringIndex) {
        if (i == 0) {
            strcpy(updatedStr, substrings[i]);
        }
        else {
            strcat(updatedStr, substrings[i]);
        }
        free(substrings[i]);
        i++;
    }

    free(substrings);
    free(myStringStartPtr);

    //printf("Updated String: %s\n", updatedStr);
    return updatedStr;
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

    // Commands will all need their first argument to just be the command again.
    // As such, we manually set that redundancy now.
    currInput->args[0] = calloc(strlen(token) + 1, sizeof(char));   // Allocate for the pointer to the argument string
    strcpy(currInput->args[0], token);
    argsIndex++;

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

    currInput->argCount = argsIndex;
    
    // realloc the args to a smaller size to fit the number of args
    currInput->args = (char**)realloc(currInput->args, argsIndex * sizeof(char*));

    // We reset the fork counter here (our failsafe), since receiving input means the program is operating normally
    resetForkBombCounter();

    return currInput;
}

