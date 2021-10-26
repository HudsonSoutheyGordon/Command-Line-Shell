#ifndef INPUTHANDLER_H_INCLUDED
#define INPUTHANDLER_H_INCLUDED

typedef struct parsedInput {

    char* command;
    char** args;
    int argCount;
    char* input;
    char* output;
    bool isBG;
    bool error;

} ParsedInput;

ParsedInput* parseInput(char* inputStr);
char* pidExpansion(char* inputStr);

#endif