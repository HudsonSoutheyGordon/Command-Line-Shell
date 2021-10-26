#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

void utilPrintf(char* str);
void freeParsedInput(ParsedInput* pi);
pid_t forkSafe(void);
void resetForkBombCounter(void);

#endif