#ifndef _INOUT_H_
#define _INOUT_H_
#include"systemd.h"
#include"loadsave.h"

void writesameln(char *line) FASTCALL;
void writeln(char* line) FASTCALL;
unsigned int readln(void);
char *eatcr(char *s) FASTCALL;
void interrogationAndAnalysis(void);
void clear(void);

#ifdef NROW
void zeror(void);
#endif
#ifdef DEFINEWTR
void wtr(const char *s) FASTCALL;
#endif

#endif