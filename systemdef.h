#ifndef __SYSTEMDEF_H__
#define __SYSTEMDEF_H__

#include<time.h>

#define NCOL 80
#define BUFFERSIZE 256

#define waitscreen()

// The number of columns of the screen
#define NCOL 80

#define inputtxt() printf("\033[1m\x1b[32m\33[40m")
#define evidence1() printf("\033[1m\x1b[31m\33[40m")
#define evidence2() printf("\033[0m\x1b[93m\33[40m")
#define normaltxt() printf("\033[0m\x1b[36m\33[40m")
#define tab() printf("\t")
#define wait1s()    {unsigned int retTime = time(0) + 1;while (time(0) < retTime);}
#define init_term() {normaltxt();printf("\n\n");}

#define leave() printf("\033[0m\n\n")

#endif