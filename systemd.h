#ifndef __SYSTEMDEF_H__
#define __SYSTEMDEF_H__

/*  This file shows an example of how to tailor the different macros so that
    the program can be compiled on modern machines (ANSI terminal) with gcc,
    as well as on vintage Commodores by using Cc65 and many Z80 machines with
    Z88dk.

    Code should be self explicative. For exemple C128 option is active, the
    chunk of #define for the C128 is selected

    NOTE: choose all buffer sizes less than 256.

    CV_IS_A_FUNCTION is a macro that if defined makes sort that a function is
        called to check a thing like "verb==v". With some compilers it may be
        an advantage (e.g. cc65) in terms of size, while it's not the case 
        for other compilers (e.f. Z88dk).
*/

#include<time.h>

#ifdef C128  /* Definitions to be used for the Commodore 128 computer */

    #include<conio.h>
    #include<stdio.h>

    #define BUFFERSIZE 128
    #define B_SIZE 160
    #define CV_IS_A_FUNCTION

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

    #define switch80col "\x1Bx\x0E"

    /* Macro to wait for a key */
    #define waitkey() cgetc()

    /* Define the style of the input text */
    #define inputtxt() fputs(green, stdout)

    /* Define the style of the first evidenced text */
    #define evidence1() fputs(red, stdout)

    /* Define the style of the second evidenced text */
    #define evidence2() fputs(yellow, stdout)

    /* Define the style of the third evidenced text */
    #define evidence3() fputs(pink, stdout)

    /* Define the style of the normal text */
    #define normaltxt() fputs(cyan, stdout)

    /* Clear the screen */
    #define cls() clrscr()

    /* Wait for one second */
    #define wait1s()    {unsigned int retTime = time(0) + 1;while (time(0) < \
        retTime);}

    /* Init the terminal */
    #define PT80COL 215
    #define PTFST 53296
    #define init_term() {\
        if (*(char*)PT80COL==0) {\
            fputs(switch80col, stdout);\
        }\
        *(char*)PTFST=1;\
        clrscr();\
        normaltxt();\
    }

    /* Prepare the terminal to leave the program execution. */
    #define leave() fputs(cyan, stdout)

#elif defined(PET)  /* Definitions to be used for the Commodore PET computer 
    with a 80-column display */

    #include<conio.h>
    #include<stdio.h>

    #define BUFFERSIZE 128
    #define B_SIZE 160
    #define CV_IS_A_FUNCTION

    #define SHIFTPETSCII \
        if((c>=0x61 && c<=0x7A)) c^=0x20; else if((c>=0x41 && c<=0x5A)) c|=0x80

    #define waitscreen()

    // The number of columns of the screen
    #define NCOL 80

    #define green       ""
    #define red         ""
    #define cyan        ""
    #define blue        ""
    #define yellow      ""
    #define pink        ""

    #define switch80col ""

    /* Macro to wait for a key */
    #define waitkey() cgetc()

    /* Define the style of the input text */
    #define inputtxt()

    /* Define the style of the first evidenced text */
    #define evidence1() PUTC(18)

    /* Define the style of the second evidenced text */
    #define evidence2()

    /* Define the style of the third evidenced text */
    #define evidence3()

    /* Define the style of the normal text */
    #define normaltxt() PUTC(146)

    /* Clear the screen */
    #define cls() clrscr()

    /* Wait for one second */
    #define wait1s()    {unsigned int retTime = time(0) + 1;while (time(0) < \
        retTime);}

    /* Init the terminal */

    #define init_term() {\
//        char i;\
        clrscr();\
        normaltxt();\
//        for(i=0;i<256;++i) putc((i), stdout); \
    }

    /* Prepare the terminal to leave the program execution. */
    #define leave()
    
#elif defined(PET40)  /* Definitions to be used for the Commodore PET computer
    with a 40-column display */

    #include<conio.h>
    #include<stdio.h>
    #define CV_IS_A_FUNCTION

    #define BUFFERSIZE 128
    #define B_SIZE 160

    #define SHIFTPETSCII \
        if((c>=0x61 && c<=0x7A)) c^=0x20; else if((c>=0x41 && c<=0x5A)) c|=0x80

    #define waitscreen()

    // The number of columns of the screen
    #define NCOL 40

    #define green       ""
    #define red         ""
    #define cyan        ""
    #define blue        ""
    #define yellow      ""
    #define pink        ""

    #define switch80col ""

    /* Macro to wait for a key */
    #define waitkey() cgetc()

    /* Define the style of the input text */
    #define inputtxt()

    /* Define the style of the first evidenced text */
    #define evidence1() PUTC(18)

    /* Define the style of the second evidenced text */
    #define evidence2()

    /* Define the style of the third evidenced text */
    #define evidence3()

    /* Define the style of the normal text */
    #define normaltxt() PUTC(146)

    /* Clear the screen */
    #define cls() clrscr()

    /* Wait for one second */
    #define wait1s()    {unsigned int retTime = time(0) + 1;while (time(0) < \
        retTime);}

    /* Init the terminal */

    #define init_term() {\
//        char i;\
        clrscr();\
        normaltxt();\
//        for(i=0;i<256;++i) putc((i), stdout); \
    }

    /* Prepare the terminal to leave the program execution. */
    #define leave()

#elif defined(C64)

    #include<conio.h>
    #include <stdio.h>
    #define CV_IS_A_FUNCTION

    #define BUFFERSIZE 40
    #define B_SIZE 80

    #define SHIFTPETSCII \
        if((c>=0x41 && c<=0x5A)||(c>=0x61 && c<=0x7A)) c^=0x20

    #define waitscreen()

    /* The number of columns of the screen */
    #define NCOL 40
    /* The number of available rows of the screen. If undefined, it is
       not checked
    */
    #define NROW 21
    extern unsigned char rowc;

    #define green       "\x1E"
    #define red         "\x1C"
    #define cyan        "\x9F"
    #define blue        "\x1F"
    #define yellow      "\x9E"
    #define pink        "\x96"


    /* Macro to wait for a key */
    #define waitkey() cgetc(); rowc=0

    /* Define the style of the input text */
    #define inputtxt() fputs(green, stdout)

    /* Define the style of the first evidenced text */
    #define evidence1() fputs(red, stdout)

    /* Define the style of the second evidenced text */
    #define evidence2() fputs(yellow, stdout)

    /* Define the style of the third evidenced text */
    #define evidence3() fputs(pink, stdout)

    /* Define the style of the normal text */
    #define normaltxt() fputs(cyan, stdout)

    /* Clear the screen */
    #define cls() clrscr();zeror()

    /* Wait for one second */
    #define wait1s()    {}
    #define PTRBRD 53280U
    #define PTRCLR 53281U
    /* Init the terminal */
    #define POKE(addr,val)     (*(unsigned char*) (addr) = (val))

    // Restore default VIC-II config (lower case)
    // This is useful if there is loader that goes in a graphic mode.

    #define init_term() {\
        *(char*)PTRBRD = 0x00;\
        *(char*)PTRCLR = 0x00;\
        clrscr();\
        normaltxt();\
        POKE(56578U, 63);\
        POKE(56576U, 151);\
        POKE(53272U, 23);\
        POKE(53265U, 27);\
    }

    /* Prepare the terminal to leave the program execution. */
    #define leave() asm("jmp $FCE2")

#elif defined(PLUS4)

    #include<conio.h>
    #include <stdio.h>
    #define CV_IS_A_FUNCTION

    #define BUFFERSIZE 80
    #define B_SIZE 240

    #define SHIFTPETSCII \
        if((c>=0x41 && c<=0x5A)||(c>=0x61 && c<=0x7A)) c^=0x20

    #define waitscreen()

    /* The number of columns of the screen */
    #define NCOL 40
    /* The number of available rows of the screen. If undefined, it is
       not checked
    */
    #define NROW 21
    extern unsigned char rowc;

    #define green       "\x1E"
    #define red         "\x1C"
    #define cyan        "\x9F"
    #define blue        "\x1F"
    #define yellow      "\x9E"
    #define pink        "\x96"


    /* Macro to wait for a key */
    #define waitkey() cgetc(); rowc=0

    /* Define the style of the input text */
    #define inputtxt() fputs(green, stdout)

    /* Define the style of the first evidenced text */
    #define evidence1() fputs(red, stdout)

    /* Define the style of the second evidenced text */
    #define evidence2() fputs(yellow, stdout)

    /* Define the style of the third evidenced text */
    #define evidence3() fputs(pink, stdout)

    /* Define the style of the normal text */
    #define normaltxt() fputs(cyan, stdout)

    /* Clear the screen */
    #define cls() clrscr();zeror()


    /* Wait for one second */
    #define wait1s()    {}
    #define PTRCLR 65301U
    /* Init the terminal */
    #define init_term() {\
        *(char*)PTRCLR = 0x00;\
        clrscr();\
        normaltxt();\
    }

    /* Prepare the terminal to leave the program execution. */
    #define leave() cputs(cyan)
#elif defined(CX16)

    #include <stdio.h>

    #define BUFFERSIZE 80
    #define B_SIZE 240
    #define CV_IS_A_FUNCTION

    #define SHIFTPETSCII \
        if((c>=0x41 && c<=0x5A)||(c>=0x61 && c<=0x7A)) c^=0x20

    #define waitscreen()

    /* The number of columns of the screen */
    #define NCOL 80
    /* The number of available rows of the screen. If undefined, it is
       not checked
    */
    #define NROW 45
    extern unsigned char rowc;

    #define green       "\x1E"
    #define red         "\x1C"
    #define cyan        "\x9F"
    #define blue        "\x1F"
    #define yellow      "\x9E"
    #define pink        "\x96"


    #define waitkey() getchar()
    #define inputtxt(green)
    #define evidence1(red)
    #define evidence2(yellow)
    #define evidence3(pink)
    #define cls()

    #define normaltxt(cyan)
    #define tab() printf("\t")
    #define wait1s()    {unsigned int retTime = time(0) + 1;while (time(0) < \
        retTime);}
    #define init_term() {\
        normaltxt();printf("\n\n");}

    #define leave()

#elif defined(VIC20)

    #include<conio.h>
    #include<stdio.h>

    #define BUFFERSIZE 44
    #define B_SIZE 88
    #define CV_IS_A_FUNCTION

    #define SHIFTPETSCII \
        if((c>=0x41 && c<=0x5A)||(c>=0x61 && c<=0x7A)) c^=0x20

    #define waitscreen()

    // The number of columns of the screen
    #define NCOL 22
    /* The number of available rows of the screen. If undefined, it is
       not checked
    */
    #define NROW 19
    extern unsigned char rowc;

    #define green       "\x1E"
    #define red         "\x1C"
    #define cyan        "\x9F"
    #define blue        "\x1F"
    #define yellow      "\x9E"
    #define pink        "\x9C"

    /* Macro to wait for a key */
    #define waitkey() cgetc(); rowc=0

    /* Define the style of the input text */
    #define inputtxt() fputs(green, stdout)

    /* Define the style of the first evidenced text */
    #define evidence1() fputs(red, stdout)

    /* Define the style of the second evidenced text */
    #define evidence2() fputs(yellow, stdout)

    /* Define the style of the third evidenced text */
    #define evidence3() fputs(pink, stdout)

    /* Define the style of the normal text */
    #define normaltxt() fputs(cyan, stdout)

    /* Clear the screen */
    #define cls() clrscr();zeror()

    /* Wait for one second */
    #define wait1s()    {}
    #define PTRCLR 36879
    /* Init the terminal */
    #define init_term() {\
        *(char*)PTRCLR = 0x08;\
        clrscr();\
        normaltxt();\
    }

    /* Prepare the terminal to leave the program execution. */
    #define leave() asm("jmp $FD22")

#elif defined(VIC20_40c)

    #include"vic40col.h"

    #define BUFFERSIZE 37
    #define B_SIZE 80
    #define CV_IS_A_FUNCTION

    #define SHIFTPETSCII \
        if((c>=0x41 && c<=0x5A)||(c>=0x61 && c<=0x7A)) c^=0x20

    #define waitscreen()

    // The number of columns of the screen
    #define NCOL 40
    /* The number of available rows of the screen. If undefined, it is
       not checked
    */
    #define NROW 21
    extern unsigned char rowc;

    /* Macro to wait for a key */
    #define waitkey() cgetc40ch(); rowc=0

    /* Define the style of the input text */
    #define inputtxt() 

    /* Define the style of the first evidenced text */
    #define evidence1() negative();
    
    /* Define the style of the second evidenced text */
    #define evidence2() 

    /* Define the style of the third evidenced text */
    #define evidence3() negative();

    /* Define the style of the normal text */
    #define normaltxt() positive();

    /* Clear the screen */
    #define cls() clrscr();

    #define PUTC(c) putc40ch(c)
    #define PUTS(s) puts40ch(s)
    #define GETS(b, si) gets40ch(b,si)

    /* Wait for one second */
    #define wait1s()    {}
    #define PTRCLR 36879
    #define RESET 64802
    
    /* Init the terminal */
    #define init_term() {\
        *(char*)PTRCLR = 0x08;\
        initGraphic();\
        normaltxt();\
    }

    /* Prepare the terminal to leave the program execution. */
    #define leave() asm("jmp $FD22")

#elif defined(SPECTRUM)

    #include <spectrum.h>
    #include <stdio.h>
    #define BUFFERSIZE 128
    #define B_SIZE 192
    #define EFFSHORTINDEX unsigned int

    #define PUTS(s) wtr(s)
    #define DEFINEWTR
    #define waitscreen()

    // The number of columns of the screen
    #define NCOL 64
    #define NROW 19
    extern unsigned char rowc;

    #define waitkey() getchar(); rowc=0
    #define inputtxt()  fputs("\x1b[0m\x1b[32m\x1b[47m", stdout)
    #define evidence1() fputs("\x1b[1m\x1b[37m\x1b[41m", stdout)
    #define evidence2() fputs("\x1b[0m\x1b[34m\x1b[47m", stdout)
    #define evidence3() fputs("\x1b[0m\x1b[35m\x1b[47m", stdout)
    #define cls()

    #define normaltxt() fputs("\x1b[0m\x1b[30m\x1b[47m", stdout)
    #define tab() fputs("\t")
    #define wait1s() \
        {unsigned int retTime = time(0) + 1;while (time(0) < retTime);}
    #define init_term() {fputs("\x1b[2J", stdout);normaltxt();}

    #define leave() fputs("\x1b[0m\n\n", stdout);
#elif defined(RC2014)
    #include <stdio.h>
    #define BUFFERSIZE 40
    #define B_SIZE 40
    #define EFFSHORTINDEX unsigned int

    #define PUTS(s) wtr(s)
    #define DEFINEWTR
    #define waitscreen()

    // The number of columns of the screen
    #define NCOL 80
    //#define NROW 22
    //extern unsigned char rowc;

    #define waitkey() getchar()
    #define inputtxt()  fputs("\x1b[0m\x1b[32m\x1b[47m", stdout)
    #define evidence1() fputs("\x1b[1m\x1b[37m\x1b[41m", stdout)
    #define evidence2() fputs("\x1b[0m\x1b[34m\x1b[47m", stdout)
    #define evidence3() fputs("\x1b[0m\x1b[35m\x1b[47m", stdout)
    #define cls()

    #define normaltxt() fputs("\x1b[0m\x1b[30m\x1b[47m", stdout)
    #define tab() //fputs("\t")
    #define wait1s() // \
        {unsigned int retTime = time(0) + 1;while (time(0) < retTime);}
    #define init_term() {fputs("\x1b[2J", stdout);normaltxt();}

    #define leave() fputs("\x1b[0m\n\n", stdout);

#elif defined(M20) /* Definitions for Olivetti M20 */

    #include<stdio.h>

    #define BUFFERSIZE 256
    #define B_SIZE 240

    #define waitscreen()

    // The number of columns of the screen
    #define NCOL 80

    #define waitkey() getchar()
    #define inputtxt()
    #define evidence1()
    #define evidence2()
    #define evidence3()
    #define cls()

    #define normaltxt()
    #define tab() fputs("\t", stdout)
    #define wait1s()    {unsigned int retTime = time(0) + 1;while (time(0) < \
        retTime);}
    #define init_term() {fputs("\n\n", stdout);}

    #define leave()

#elif defined(CPC) /* Definitions for Amstrad CPC computers */

    #include<stdio.h>
    #include <sys/ioctl.h>        // required for switching the screen mode
    #include <cpc.h>

    #define BUFFERSIZE 256
    #define B_SIZE 240
    #define EFFSHORTINDEX unsigned int

    #define waitscreen()

    // The number of columns of the screen
    #define NCOL 80

    #define waitkey() getchar()
    #define inputtxt() printf("")
    #define evidence1() printf("\x18")
    #define end_evidence1() printf("\x18")
    #define evidence2() printf("")
    #define evidence3() printf("")
    #define cls() printf("\x0C")

    #define normaltxt() printf("")
    #define tab() fputs("\t", stdout)
    #define wait1s()    {unsigned int retTime = time(0) + 1;while (time(0) < \
        retTime);}
    #define init_term() {printf("\x04\x02");}
    //int mode=2;console_ioctl(IOCTL_GENCON_SET_MODE, &mode);}

    #define leave()
    /* Strangely enough, the \n char is not printed when one hits return */
    #define GETS(buffer, size) fgets((buffer),(size),stdin); PUTC('\n')

#elif defined(MSX) /* Definitions for MSX computers */
    
    #include <stdio.h>
    #define BUFFERSIZE 40
    #define B_SIZE 40

    #define PUTS(s) wtr(s)
    #define DEFINEWTR
    #define waitscreen()
    #define EFFSHORTINDEX unsigned int

    // The number of columns of the screen
    #define NCOL 32
    #define NROW 24
    extern unsigned char rowc;

    #define waitkey() getchar()
    #define inputtxt() 
    #define evidence1() 
    #define evidence2() 
    #define evidence3() 
    #define cls()

    #define normaltxt() 
    #define tab() //fputs("\t")
    #define wait1s() // \
        {unsigned int retTime = time(0) + 1;while (time(0) < retTime);}
    #define init_term() {}

    #define leave() 

#else /* Definitions for modern ANSI terminals */

    #include<stdio.h>
    #define BUFFERSIZE 256
    #define B_SIZE 240

    #define waitscreen()

    // The number of columns of the screen
    #define NCOL 80

    #define waitkey() getchar()
    #define inputtxt() printf("\033[1m\x1b[32m\33[40m")
    #define evidence1() printf("\033[1m\x1b[31m\33[40m")
    #define evidence2() printf("\033[0m\x1b[93m\33[40m")
    #define evidence3() printf("\033[0m\x1b[95m\33[40m")
    #define cls()

    #define normaltxt() printf("\033[0m\x1b[36m\33[40m")
    #define tab() printf("\t")
    #define wait1s()    {unsigned int retTime = time(0) + 1;while (time(0) < \
        retTime);}
    #define init_term() {\
        printf( "This terminal does not support ANSI codes."\
        "\033[80D"\
        "You'll see garbage chars on the screen. If you use MS-DOS, add "\
        "\033[80D"\
        "DEVICE=DOS\\ANSI.SYS to your CONFIG.SYS file"\
        "\033[80D"\
        "It's supported since MS-DOS 2.0, so no excuses."\
"\033[80D                                                               ");\
        normaltxt();printf("\n\n");}

    #define leave() printf("\033[0m\n\n")

#endif

#ifndef EFFSHORTINDEX
    #define EFFSHORTINDEX unsigned char
#endif

#ifndef SHIFTPETSCII
    #define SHIFTPETSCII
#endif

#ifndef end_evidence1
    #define end_evidence1()
#endif

#ifndef PUTC
    #define PUTC(c) putc((c), stdout)
#endif
#ifndef PUTS
    #define PUTS(s) fputs((s), stdout)
#endif
#ifndef GETS
    #define GETS(buffer, size) fgets((buffer),(size),stdin);
#endif

#endif