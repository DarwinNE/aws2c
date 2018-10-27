#ifndef __SYSTEMDEF_H__
#define __SYSTEMDEF_H__

/*  This file shows an example of how to tailor the different macros so that
    the program can be compiled on modern machines (ANSI terminal) with gcc,
    as well as on vintage Commodores by using Cc65.

    If the C128 option is active, the code for the C128
*/

#include<time.h>

#ifdef C128  /* Definitions to be used for the Commodore 128 computer */

#include<conio.h>

#define BUFFERSIZE 128

#define SHIFTPETSCII \
    if((c>=0x41 && c<=0x5A)||(c>=0x61 && c<=0x7A)) c^=0x20

#define waitscreen()

// The number of columns of the screen
#define NCOL 80

#define green       "\x1E"
#define red         "\x1C"
#define cyan        "\x9F"
#define blue        "\x1F"
#define yellow      "\x9E"
#define pink        "\x96"

#define cls         "\x93"

#define switch80col "\x1Bx\x0E"

/* Macro to wait for a key */
#define waitkey() cgetc()

/* Define the style of the input text */
#define inputtxt() printf(green)

/* Define the style of the first evidenced text */
#define evidence1() printf(red)

/* Define the style of the second evidenced text */
#define evidence2() printf(yellow)

/* Define the style of the third evidenced text */
#define evidence3() printf(pink)

/* Define the style of the normal text */
#define normaltxt() printf(cyan)

/* Write a tabulation (it can be adapted to screen width). */
#define tab() printf("        ")

/* Wait for one second */
#define wait1s()    {unsigned int retTime = time(0) + 1;while (time(0) < \
    retTime);}

/* Init the terminal */
#define PT80COL 215
#define PTFST 53296
#define init_term() {\
    if (*(char*)PT80COL==0) {\
        printf(switch80col);\
    }\
    *(char*)PTFST=1;\
    cputs(cls);\
    normaltxt();\
}

/* Prepare the terminal to leave the program execution. */
#define leave() printf(cyan)

#elif defined(C64)

#include<conio.h>

#define BUFFERSIZE 128

#define SHIFTPETSCII \
    if((c>=0x41 && c<=0x5A)||(c>=0x61 && c<=0x7A)) c^=0x20

#define waitscreen()

// The number of columns of the screen
#define NCOL 40

#define green       "\x1E"
#define red         "\x1C"
#define cyan        "\x9F"
#define blue        "\x1F"
#define yellow      "\x9E"
#define pink        "\x96"

#define cls         "\x93"


/* Macro to wait for a key */
#define waitkey() cgetc()

/* Define the style of the input text */
#define inputtxt() printf(green)

/* Define the style of the first evidenced text */
#define evidence1() printf(red)

/* Define the style of the second evidenced text */
#define evidence2() printf(yellow)

/* Define the style of the third evidenced text */
#define evidence3() printf(pink)

/* Define the style of the normal text */
#define normaltxt() printf(cyan)

/* Write a tabulation (it can be adapted to screen width). */
#define tab() printf("    ")

/* Wait for one second */
#define wait1s()    {}
#define PTRBRD 53280
#define PTRCLR 53281
/* Init the terminal */
#define init_term() {\
    *(char*)PTRBRD = 0x00;\
    *(char*)PTRCLR = 0x00;\
    clrscr();\
    normaltxt();\
}

/* Prepare the terminal to leave the program execution. */
#define leave() cputs(cyan)

#elif defined(VIC20)

#include<conio.h>

#define BUFFERSIZE 128

#define SHIFTPETSCII \
    if((c>=0x41 && c<=0x5A)||(c>=0x61 && c<=0x7A)) c^=0x20

#define waitscreen()

// The number of columns of the screen
#define NCOL 22

#define green       "\x1E"
#define red         "\x1C"
#define cyan        "\x9F"
#define blue        "\x1F"
#define yellow      "\x9E"
#define cls         "\x93"


/* Macro to wait for a key */
#define waitkey() cgetc()

/* Define the style of the input text */
#define inputtxt() printf(green)

/* Define the style of the first evidenced text */
#define evidence1() printf(red)

/* Define the style of the second evidenced text */
#define evidence2() printf(yellow)

/* Define the style of the normal text */
#define normaltxt() printf(cyan)

/* Write a tabulation (it can be adapted to screen width). */
#define tab() printf("    ")

/* Wait for one second */
#define wait1s()    {}
#define PTRCLR 36879
/* Init the terminal */
#define init_term() {\
    *(char*)PTRCLR = 0x08;\
    printf(cls);\
    normaltxt();\
}

/* Prepare the terminal to leave the program execution. */
#define leave() cputs(cyan)

#else /* Definitions for modern ANSI terminals */

#define NCOL 80
#define BUFFERSIZE 256

#define SHIFTPETSCII 

#define waitscreen()

// The number of columns of the screen
#define NCOL 80

#define waitkey() getchar()
#define inputtxt() printf("\033[1m\x1b[32m\33[40m")
#define evidence1() printf("\033[1m\x1b[31m\33[40m")
#define evidence2() printf("\033[0m\x1b[93m\33[40m")
#define evidence3() printf("\033[0m\x1b[95m\33[40m")

#define normaltxt() printf("\033[0m\x1b[36m\33[40m")
#define tab() printf("\t")
#define wait1s()    {unsigned int retTime = time(0) + 1;while (time(0) < retTime);}
#define init_term() {normaltxt();printf("\n\n");}

#define leave() printf("\033[0m\n\n")

#endif
#endif