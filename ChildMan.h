#ifndef CHILDMAN_H_INCLUDED
#define CHILDMAN_H_INCLUDED

void externalFunc(ParsedInput* pi, int* hasBGChild, int** decodedExit);
void bgCheck(int* hasBGChild);
void toggleBG(void);
void initChildren(void);
void killAllChildren(void);

#endif