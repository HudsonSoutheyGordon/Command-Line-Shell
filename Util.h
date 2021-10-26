#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

void utilPrintf(char* str);

void debugPWD(void);
void freeParsedInput(ParsedInput* pi);
char** tokenizePATH(int* pathsLength);
pid_t forkSafe(void);
void resetForkBombCounter(void);

#endif