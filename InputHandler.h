#ifndef INPUTHANDLER_H_INCLUDED
#define INPUTHANDLER_H_INCLUDED

typedef struct parsedInput {

    char* command;
    char** args;
    char* input;
    char* output;
    bool isBg;

} ParsedInput;

ParsedInput parseInput(char* inputStr);

#endif