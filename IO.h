#ifndef IO_H_INCLUDED
#define IO_H_INCLUDED

void closeFiles(void);
void redirectInput(ParsedInput* pi);
void redirectOutput(ParsedInput* pi);

#endif