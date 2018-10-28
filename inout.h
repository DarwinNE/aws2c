#ifndef _INOUT_H_
#define _INOUT_H_
#include"systemdef.h"

void writesameln(char *line);
void writeln(char* line);
unsigned int readln(void);
void interrogationAndAnalysis(unsigned int num_of_words);
void clear(void);
#ifdef NROW
void zeror(void);
#endif

#endif