#ifndef SIGMAN_H_INCLUDED
#define SIGMAN_H_INCLUDED

void registerSigHandlers(void);
void stopIgnoreSIGINT(void);
void setSIGBlock(ParsedInput* pi);
void unblockSIGTSTP(void);
void childIgnoreSIGTSTP(void);

#endif