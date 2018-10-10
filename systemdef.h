#ifndef __SYSTEMDEF_H__
#define __SYSTEMDEF_H__

/*  This file shows an example of how to tailor the different macros so that
    the program can be compiled on modern machines (ANSI terminal) with gcc,
    as well as on vintage Commodores by using Cc65.

    If the C128 option is active, the code for the C128
*/

#include<time.h>

#ifdef C128  /* Definitions to be used for the Commodore 128 computer */

#define BUFFERSIZE 128

#define waitscreen()

// The number of columns of the screen
#define NCOL 80

#define green       "\x1E"
#define red         "\x1C"
#define cyan        "\x9F"
#define blue        "\x1F"
#define yellow      "\x9E"
#define cls         "\x93"

#define switch80col "\x1Bx\x0E"

#define inputtxt() printf(green)
#define evidence1() printf(red)
#define evidence2() printf(yellow)
#define normaltxt() printf(cyan)
#define tab() printf("        ")
#define wait1s()    {unsigned int retTime = time(0) + 1;while (time(0) < \
    retTime);}
#define init_term() {\
    char *pt80col=(char*)215;\
    char *ptFast=(char*)53296;\
    if (*pt80col==0) {\
        printf(switch80col);\
    }\
    *ptFast=1;\
    printf(cls);\
    normaltxt();\
}

#define leave() printf(cyan)


#else /* Definitions for modern ANSI terminals */

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
#endif