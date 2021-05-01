#ifndef __SYSTEMDEF_H__
#define __SYSTEMDEF_H__

/*  This file shows an example of how to tailor the different macros so that
    the program can be compiled on modern machines (ANSI terminal) with gcc,
    as well as on vintage Commodores by using Cc65 and many Z80 machines with
    Z88dk.

    Code should be self explicative. For exemple C128 option is active, the
    chunk of #define for the C128 is selected

    NOTE: choose all buffer sizes less than 256.

    inputtxt is called just before the prompt

    evidence1 is called just before writing the name of the current room.

    evidence2 is called just before the game title and the list of objects
        available in a certain room.

    evidence3 is called just before writing the directions allowed.
    
    CV_IS_A_FUNCTION is a macro that if defined makes sort that a function is
        called to check a thing like "verb==v". With some compilers it may be
        an advantage (e.g. cc65) in terms of size, while it's not the case
        for other compilers (e.f. Z88dk).
*/

#include<time.h>

/* Provide very basic load/save routines */
#define SIMPLELOAD {\
        PUTS("Enter file name: ");\
        GETS(playerInput, BUFFERSIZE);\
        loadgame(playerInput);\
    }

#define SIMPLESAVE {\
        PUTS("Enter file name: ");\
        GETS(playerInput, BUFFERSIZE);\
        savegame(playerInput);\
    }

#ifdef C128  /* Definitions to be used for the Commodore 128 computer */

    #include<conio.h>
    #include<stdio.h>

    #define BUFFERSIZE 128
    #define B_SIZE 160
    #define CV_IS_A_FUNCTION

    #define LOAD SIMPLELOAD
    #define SAVE SIMPLESAVE
    //#define FASTCALL __fastcall__
    #define FASTCALL2 __fastcall__

    #define SHIFTPETSCII \
        if((c>=0x41 && c<=0x5A)||(c>=0x61 && c<=0x7A)) c^=0x20
    #define COMMODORE8BIT

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

    #define evidence1() fputs(red, stdout)
    #define evidence2() fputs(yellow, stdout)
    #define evidence3() fputs(pink, stdout)
    #define normaltxt() fputs(cyan, stdout)
    #define cls() clrscr()

    #define wait1s()    {unsigned int retTime = time(0) + 1;while (time(0) < \
        retTime);}

    /* Init the terminal */
    #define PT80COL 215
    #define PTFST 53296L
    #ifdef ALTSPLASH
    #include"C128_splash/c128sp.h"
    #define init_term() {\
        if (*(char*)PT80COL==0) {\
            fputs(switch80col, stdout);\
        }\
        *(char*)PTFST=1;\
        showsplash();\
        fputs("\x09\x0E\x08",stdout);\
        clrscr();\
        normaltxt();\
    }
    #else
    #define init_term() {\
        if (*(char*)PT80COL==0) {\
            fputs(switch80col, stdout);\
        }\
        *(char*)PTFST=1;\
        clrscr();\
        normaltxt();\
    }
    #endif
    
    /* Prepare the terminal to leave the program execution. */
    #define leave() fputs(cyan, stdout)

#elif defined(PET)  /* Definitions to be used for the Commodore PET computer 
    with a 80-column display */

    #include<conio.h>
    #include<stdio.h>

    #define BUFFERSIZE 80
    #define B_SIZE 80
    #define CV_IS_A_FUNCTION
    #define LOAD SIMPLELOAD
    #define SAVE SIMPLESAVE

    #define SHIFTPETSCII \
        if((c>=0x61 && c<=0x7A)) c^=0x20; else if((c>=0x41 && c<=0x5A)) c|=0x80
    #define COMMODORE8BIT

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

    #define init_term()

    /* Prepare the terminal to leave the program execution. */
    #define leave()
    
#elif defined(PET40)  /* Definitions to be used for the Commodore PET computer
    with a 40-column display */

    #include<conio.h>
    #include<stdio.h>

    #define BUFFERSIZE 80
    #define B_SIZE 80
    #define CV_IS_A_FUNCTION
    #define LOAD SIMPLELOAD
    #define SAVE SIMPLESAVE

    #define SHIFTPETSCII \
        if((c>=0x61 && c<=0x7A)) c^=0x20; else if((c>=0x41 && c<=0x5A)) c|=0x80
    #define COMMODORE8BIT

    #define waitscreen()

    // The number of columns of the screen
    #define NCOL 40
    #define NROW 24

    #define green       ""
    #define red         ""
    #define cyan        ""
    #define blue        ""
    #define yellow      ""
    #define pink        ""

    #define switch80col ""

    /* Macro to wait for a key */
    #define waitkey() cgetc(); rowc=0

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

    #define init_term()

    /* Prepare the terminal to leave the program execution. */
    #define leave()

#elif defined(C64)

    #include<conio.h>
    #include <stdio.h>
    #define CV_IS_A_FUNCTION

    #define BUFFERSIZE 40
    #define B_SIZE 80

    #define LOAD SIMPLELOAD
    #define SAVE SIMPLESAVE

    #define SHIFTPETSCII \
        if((c>=0x41 && c<=0x5A)||(c>=0x61 && c<=0x7A)) c^=0x20
    #define COMMODORE8BIT

    #define waitscreen()

    /* The number of columns of the screen */
    #define NCOL 40
    /* The number of available rows of the screen. If undefined, it is
       not checked
    */
    #define NROW 21

    #define _C64_ADDRS_NORMAL 23
    #ifdef ALTFONT
    #define _C64_ADDRS_F 19
    #else
    #define _C64_ADDRS_F _C64_ADDRS_NORMAL
    #endif

    #define red         "\x1C\x1C"   // If just one char, it goes crazy (why?)
    #define cyan        "\x9F"
    #define green       "\x1E"
    #define blue        "\x1F"
    #define yellow      "\x9E"
    #define pink        "\x96"

    #define normaltxt() fputs(cyan, stdout)

    /* Macro to wait for a key */
    #define waitkey() cgetc(); rowc=0

    /* Define the style of the input text */
    #define inputtxt() fputs(green, stdout)

    #define evidence1() fputs(red, stdout)
    #define evidence2() fputs(yellow, stdout)
    #define evidence3() fputs(pink, stdout)

    /* Clear the screen */
    #define cls() clrscr();zeror()

    /* Wait for one second */
    #define wait1s()    {}
    #define PTRBRD 53280U
    #define PTRCLR 53281U
    /* Init the terminal */
    #define POKE(addr,val)     (*(unsigned char*) (addr) = (val))
    #define PEEK(addr)     (*(unsigned char*) (addr))
    // Restore default VIC-II config (lower case)
    // This is useful if there is loader that goes in a graphic mode.


    #define init_term() {\
        *(char*)PTRBRD = 0x00;\
        *(char*)PTRCLR = 0x00;\
        POKE(56578U, 63);\
        POKE(56576U, 151);\
        if(PEEK(0x52)==0) POKE(53272U, _C64_ADDRS_F); else POKE(53272U, _C64_ADDRS_NORMAL);\
        POKE(53265U, 27);\
        clrscr();\
        normaltxt();\
    }


    /* Prepare the terminal to leave the program execution. */
    #define leave() asm("jmp $FCE2")

#elif defined(PLUS4)

    #include<conio.h>
    #include <stdio.h>
    #define CV_IS_A_FUNCTION

    #define BUFFERSIZE 80
    #define B_SIZE 240

    #define LOAD SIMPLELOAD
    #define SAVE SIMPLESAVE

    #define SHIFTPETSCII \
        if((c>=0x41 && c<=0x5A)||(c>=0x61 && c<=0x7A)) c^=0x20
    #define COMMODORE8BIT

    #define waitscreen()

    /* The number of columns of the screen */
    #define NCOL 40
    /* The number of available rows of the screen. If undefined, it is
       not checked
    */
    #define NROW 21

    #define green     "\x1E"
    #define red       "\x1C"
    #define cyan      "\x9F"
    #define blue      "\x1F"
    #define yellow    "\x9E"
    #define pink      "\x96"


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
    #include "commanderX16.h"
    
    #define PUTC(c) putc_x16(c)
    #define PUTS(s) puts_x16(s)
    #define GETS(buffer, size) gets_x16((buffer), (size))

    
    #define BUFFERSIZE 80
    #define B_SIZE 240
    #define CV_IS_A_FUNCTION

    #define LOAD SIMPLELOAD
    #define SAVE SIMPLESAVE

    #define waitscreen()

    #define waitkey() console_paging();getc_x16()
    #define inputtxt() console_paging();PUTC(ATTR_BOLD);PUTC(COLOR_BLUE)
    #define evidence1() PUTC(ATTR_BOLD); PUTC(COLOR_RED)
    #define evidence2() PUTC(COLOR_GREEN)
    #define evidence3() PUTC(ATTR_ITALICS); PUTC(COLOR_PINK)
    #define cls()

    #define normaltxt() PUTC(ATTR_RESET); PUTC(COLOR_BLACK)

    #define tab() printf("\t")
    #define wait1s()    {unsigned int retTime = time(0) + 1;while (time(0) < \
        retTime);}
    #define init_term() {init_x16();\
        normaltxt();PUTS("\n\n");}

    #define leave()

#elif defined(VIC20)

    #include<conio.h>
    #include<stdio.h>

    #define BUFFERSIZE 44
    #define B_SIZE 88
    #define CV_IS_A_FUNCTION

    #define LOAD SIMPLELOAD
    #define SAVE SIMPLESAVE

    #define SHIFTPETSCII \
        if((c>=0x41 && c<=0x5A)||(c>=0x61 && c<=0x7A)) c^=0x20
    #define COMMODORE8BIT

    #define waitscreen()

    // The number of columns of the screen
    #define NCOL 22
    /* The number of available rows of the screen. If undefined, it is
       not checked
    */
    #define NROW 19

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

    #define LOAD SIMPLELOAD
    #define SAVE SIMPLESAVE

    #define SHIFTPETSCII \
        if((c>=0x41 && c<=0x5A)||(c>=0x61 && c<=0x7A)) c^=0x20
    #define COMMODORE8BIT

    #define waitscreen()

    // The number of columns of the screen
    #define NCOL 40
    /* The number of available rows of the screen. If undefined, it is
       not checked
    */
    #define NROW 21

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
    #define BUFFERSIZE 80
    #define B_SIZE 80
    #define EFFSHORTINDEX unsigned int
    #define FASTCALL __z88dk_fastcall
    #define LOAD SIMPLELOAD
    #define SAVE SIMPLESAVE

    #define PUTS(s) wtr(s)
    #define DEFINEWTR
    #define waitscreen()

    // The number of columns of the screen
    #define NCOL 64
    #define NROW 19

    #define waitkey() getchar(); rowc=0
    #define inputtxt()  fputs("\x1b[0m\x1b[34m\x1b[47m", stdout)
    #define evidence1() fputs("\x1b[1m\x1b[37m\x1b[41m", stdout)
    #define evidence2() fputs("\x1b[0m\x1b[34m\x1b[47m", stdout)
    #define evidence3() fputs("\x1b[0m\x1b[35m\x1b[47m", stdout)
    #define cls()

    #define normaltxt() fputs("\x1b[0m\x1b[30m\x1b[47m", stdout)
    #define tab() fputs("\t")
    #define wait1s() \
        {unsigned int k,r; for(k=0;k<32000;++k) r=k>>2;}
    #define init_term() {wait1s();wait1s();fputs("\x1b[2J", stdout);normaltxt();}

    #define leave() fputs("\x1b[0m\n\n", stdout);
#elif defined(RC2014)
    #include <stdio.h>
    #define BUFFERSIZE 40
    #define B_SIZE 40
    #define EFFSHORTINDEX unsigned int
    #define FASTCALL __z88dk_fastcall

    #define waitscreen()

    // The number of columns of the screen
    #define NCOL 80

    #define waitkey() getchar()
    #define inputtxt()  fputs("\x1b[0m\x1b[32m\x1b[47m", stdout)
    #define evidence1() fputs("\x1b[1m\x1b[37m\x1b[41m", stdout)
    #define evidence2() fputs("\x1b[0m\x1b[34m\x1b[47m", stdout)
    #define evidence3() fputs("\x1b[0m\x1b[35m\x1b[47m", stdout)
    #define cls()

    #define normaltxt() fputs("\x1b[0m\x1b[30m\x1b[47m", stdout)
    #define tab() //fputs("\t")
    #define wait1s()
    #define init_term() {fputs("\x1b[2J", stdout);normaltxt();}

    #define leave() fputs("\x1b[0m\n\n", stdout);

#elif defined(M20) /* Definitions for Olivetti M20 */

    #include<stdio.h>
    #include <sys/pcos.h>

    #define BUFFERSIZE 128
    #define B_SIZE 128
    #define INTERNAL_DEF

    #define LOAD SIMPLELOAD
    #define SAVE SIMPLESAVE
    
    void m20_putc(char c);
    void wait_key(void);

    
    #define PUTS(s) _pcos_dstring(s)
    #define PUTC(c) m20_putc(c)

    #define waitscreen()

    // The number of columns of the screen
    #define NCOL 80

    #define waitkey() wait_key();
    #define inputtxt()
    #define evidence1()
    #define evidence2()
    #define evidence3()
    #define cls()

    #define normaltxt()
    #define tab() fputs("\t", stdout)
    #define wait1s()
#ifdef ALTSPLASH
    #include"m20sp.h"
    #define init_term() {showsplash();}
#else
    void set80col(void);
    #define init_term() {set80col();}
#endif

    #define leave()

#elif defined(CPC) /* Definitions for Amstrad CPC computers */

    #include<stdio.h>
    #include <cpc.h>

    #define BUFFERSIZE 255
    #define B_SIZE 240
    #define EFFSHORTINDEX unsigned int
    #define LOAD SIMPLELOAD
    #define SAVE SIMPLESAVE

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

    #define leave()
    /* Strangely enough, the \n char is not printed when one hits return */
    #define GETS(buffer, size) fgets((buffer),(size),stdin); PUTC('\n')

#elif defined(MSX) /* Definitions for MSX computers */
    #include <stdio.h>
    #include <msx.h>
    #define BUFFERSIZE 40
    #define B_SIZE 40

    #define FASTCALL __z88dk_fastcall
    #define waitscreen()
    #define EFFSHORTINDEX unsigned int
    //#define LOAD SIMPLELOAD
    //#define SAVE SIMPLESAVE

    // The number of columns of the screen
    #define NCOL 37
    #define NROW 22

    #define waitkey() getchar(); rowc=0
    #define inputtxt() 
    #define evidence1()
    #define evidence2() 
    #define evidence3() 
    #define cls()

    #define normaltxt()
    #define tab() //fputs("\t")
    #define wait1s()

    // Jump to the BIOS routines to set text mode and colours.

    #define init_term() {\
        asm("call 0x006C");\
        asm("ld a, 15"); asm("ld (0xF3E9),a");\
        asm("ld a, 1"); asm("ld (0xF3EA),a");\
        asm("ld a, 1"); asm("ld (0xF3EC),a");\
        asm("call 0x0062");\
        asm("call 0x0115");\
    }
    
    // Jump to the BIOS Reset routine.
    #define leave() {asm("call 0x0000");}
    
    #define PUTC(c) fputc_cons(c)
    #define DEFINEWTR
    #define PUTS(s) wtr(s)
    #define GETS(buffer, size) fgets_cons((buffer),(size));

#elif defined(D68kmac) /* Definitions for 68k Macintoshes */
    #include<stdio.h>
    #include<string.h>
    #include"../68kmac/SplashWindow.h"
    #define BUFFERSIZE 255
    #define B_SIZE 240
#define LOAD {\
        if(getFileName(playerInput, BUFFERSIZE, 1)!=NULL) {\
            if(playerInput[0]=='.') \
                PUTS("Invalid file name!\n");\
            else
                loadgame(playerInput)\
        }\
    }

    #define SAVE saveMac()

    #define waitscreen()

    // The number of columns of the screen
    #define NCOL 80
    #define NROW 24

    #define waitkey() disablem(); getchar(); rowc=0; enablem();
    #define inputtxt() printf("\033[1m")
    #define evidence1() printf("\033[4m")
    #define evidence2() printf("\033[1m")
    #define evidence3() printf("\033[3m")
    #define cls()

    #define normaltxt() printf("\033[0m")
    #define tab() //fputs("\t")
    #define wait1s()

    #define init_term() {splash(); printf("\033]0;" GAMEN "\007");}
    #define leave()
    #define GETS(buffer, size) fflush(stdout);\
        fgets((buffer),(size),stdin)

#elif defined(AMIGA)
        
    #include <proto/exec.h>
    #include <proto/dos.h>
    #include <stdio.h>
   // #include "amigastub.h"

   // extern int __nocommandline;
   // extern int __oslibversion;

    #define LOAD SIMPLELOAD
    #define SAVE SIMPLESAVE

    #define BUFFERSIZE 255
    #define B_SIZE 240

    #define waitscreen()
    // The number of columns of the screen
    #define NCOL 60
    #define NROW 18

  
    #define PUTC(c) printf("%c",c)
    #define PUTS(s) printf("%s",s)
    #define GETS(buffer, size) gets(buffer) // Meh!!! I don't like that.

    #define waitkey() getchar(); rowc=0
    #define inputtxt() PUTS("\033[1m\033[31m")
    #define evidence1() PUTS("\033[1m\033[4m")
    #define evidence2() PUTS("\033[1m\033[32m")
    #define evidence3() PUTS("\033[3m\033[33m")
    #define cls()

    #define normaltxt() PUTS("\033[0m\033[31m")
    #define tab() PUTS("\t")
    #define wait1s()    {unsigned int retTime = time(0) + 1;while (time(0) < \
        retTime);}
    #define init_term() normaltxt()

    #define leave()
#elif defined(SAMC) /* Definitions for Sam Coupé */

    #include<stdio.h>

    #define BUFFERSIZE 128  
    #define B_SIZE 120

    #define waitscreen()
    #define LOAD SIMPLELOAD
    #define SAVE SIMPLESAVE
    // The number of columns of the screen
    #define NCOL 32
    #define NROW 20

    #define waitkey() getchar(); rowc=0
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
#elif defined(ATARI) /* Definitions for Atari 8-bit computers */

    #include<atari.h>
    #include<stdio.h>

    #define BUFFERSIZE 128
    #define B_SIZE 120

    #define waitscreen()
    #define LOAD SIMPLELOAD
    #define SAVE SIMPLESAVE
    // The number of columns of the screen
    #ifndef NCOL
        #define NCOL 40
    #endif
    #ifndef NROW
        #define NROW 22
    #endif

    extern int invert;
    // This is not a Commodore computer, but the macro is executed at the
    // right moment to convert the carriage return from 13 (0x0D) to 155 (0x9B)
    #define SHIFTPETSCII \
        {if(c==0x0A) c=0x9B; else if(invert&&c!=0) c^=0x80;}


    #define waitkey() getchar(); rowc=0
    #define inputtxt() 
    #define evidence1() invert=1
    #define evidence2()
    #define evidence3() invert=1
    #define cls()

    #define normaltxt() invert=0
    #define tab() fputs("\t", stdout)
    #define wait1s()    {unsigned int retTime = time(0) + 1;while (time(0) < \
        retTime);}
    #define init_term()
    #define leave()

#elif defined(NOANSI) /* Definitions for a plain text terminal, with no ansi
                         support */

    #include<stdio.h>

    #define BUFFERSIZE 128  
    #define B_SIZE 120

    #define waitscreen()
    #define LOAD SIMPLELOAD
    #define SAVE SIMPLESAVE
    // The number of columns of the screen
    #ifndef NCOL
        #define NCOL 80
    #endif
    #ifndef NROW
        #define NROW 24
    #endif

    #define waitkey() getchar(); rowc=0
    #define inputtxt()
    #define evidence1()
    #define evidence2()
    #define evidence3()
    #define cls()

    #define normaltxt()
    #define tab() fputs("\t", stdout)
    #define wait1s()    {unsigned int retTime = time(0) + 1;while (time(0) < \
        retTime);}

    #ifdef ALTSPLASH
        #include"CGASP.H"
        #define init_term() {showsplash();}
    #else
        #define init_term() {}
    #endif
    #define leave()

#elif defined(MSDOS) /* Definitions for MS-DOS terminals*/
    #include<stdio.h>
    #define BUFFERSIZE 255
    #define B_SIZE 240

    #define LOAD SIMPLELOAD
    #define SAVE SIMPLESAVE

    #define waitscreen()
    // The number of columns of the screen
    #define NCOL 80
    #define NROW 21

    #define waitkey() getchar(); rowc=0

    #ifdef NOANSI
        #define inputtxt()
        #define evidence1()
        #define evidence2()
        #define evidence3()
        #define cls()
    #else
        #define inputtxt() printf("\033[1m\x1b[32m\33[40m")
        #define evidence1() printf("\033[1m\x1b[31m\33[40m")
        #define evidence2() printf("\033[0m\x1b[93m\33[40m")
        #define evidence3() printf("\033[0m\x1b[95m\33[40m")
    #endif

    #define cls()

    #define normaltxt() printf("\033[0m\x1b[36m\33[40m")
    #define tab() printf("\t")
    #define wait1s()    {unsigned int retTime = time(0) + 1;while (time(0) < \
        retTime);}
    #ifdef ALTSPLASH
        #include"CGASP.H"
        #define init_term() {showsplash();\
            printf( "This terminal does not support ANSI codes."\
            "\033[80D"\
            "You'll see garbage chars on the screen. If you use MS-DOS, add "\
            "\033[80D"\
            "DEVICE=DOS\\ANSI.SYS to your CONFIG.SYS file"\
            "\033[80D"\
    "\033[80D                                                               ");\
            normaltxt();printf("\n\n");}
    #else
        #define init_term() {\
            printf( "This terminal does not support ANSI codes."\
            "\033[80D"\
            "You'll see garbage chars on the screen. If you use MS-DOS, add "\
            "\033[80D"\
            "DEVICE=DOS\\ANSI.SYS to your CONFIG.SYS file"\
            "\033[80D"\
    "\033[80D                                                               ");\
            normaltxt();printf("\n\n");}
    #endif
    #define leave() printf("\033[0m\n\n")
#elif defined(ATARI_ST) /* Definitions for ATARI-ST */
    #include<stdio.h>
    #include<mint/sysbind.h>
    #define BUFFERSIZE 255
    #define B_SIZE 240

    #define LOAD SIMPLELOAD
    #define SAVE SIMPLESAVE

    #define waitscreen()

    #define evidence1() PUTS("\x1b" "b\x01" "\x1b" "p")

    // The number of columns of the screen
    #ifndef LOWRES
        #define NCOL 80
        #define inputtxt()  
        #define evidence2() 
        #define evidence3() 
    #else
        #define NCOL 40
        #define inputtxt() PUTS("\x1b" "b\x08")
        #define evidence1() PUTS("\x1b" "b\x01" "\x1b" "p")
        #define evidence2() PUTS("\x1b" "b\x0C")
        #define evidence3() PUTS("\x1b" "b\x04")
    #endif
    #define NROW 23

    #define PUTC(c) Cconout(c)
    #define PUTS(s) Cconws(s)

    #define waitkey() getchar()
    #define cls()
    
    #ifdef ALTSPLASH
        extern  void  pic_display();
        #define init_term() pic_display()
    #else
        #define init_term()
    #endif
    #define normaltxt() PUTS("\x1b" "b\x0f")
    #define end_evidence1() PUTS("\x1b" "b\x0f" "\x1b" "q")
    #define tab() printf("\t")
    #define wait1s()    {unsigned int retTime = time(0) + 1;while (time(0) < \
        retTime);}
    #define leave() PUTS("\x1b" "E")
    
#else /* Definitions for modern ANSI terminals */
    #include<stdio.h>
    #define BUFFERSIZE 255
    #define B_SIZE 240

    #define LOAD SIMPLELOAD
    #define SAVE SIMPLESAVE

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
#ifdef ALTSPLASH
    #include"CGASP.H"
    #define init_term() {showsplash();\
        printf( "This terminal does not support ANSI codes."\
        "\033[80D"\
        "You'll see garbage chars on the screen. If you use MS-DOS, add "\
        "\033[80D"\
        "DEVICE=DOS\\ANSI.SYS to your CONFIG.SYS file"\
        "\033[80D"\
"\033[80D                                                               ");\
        normaltxt();printf("\n\n");}
#else
    #define init_term() {\
        printf( "This terminal does not support ANSI codes."\
        "\033[80D"\
        "You'll see garbage chars on the screen. If you use MS-DOS, add "\
        "\033[80D"\
        "DEVICE=DOS\\ANSI.SYS to your CONFIG.SYS file"\
        "\033[80D"\
"\033[80D                                                               ");\
        normaltxt();printf("\n\n");}
#endif
    #define leave() printf("\033[0m\n\n")
#endif

#ifndef EFFSHORTINDEX
    #define EFFSHORTINDEX unsigned char
#endif

#ifdef NROW
    extern EFFSHORTINDEX rowc;
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

#ifndef LOAD
    #define LOAD PUTS("Loading is not supported on this platform.")
#endif

#ifndef SAVE
    #define SAVE PUTS("Saving is not supported on this platform.")
#endif

#ifndef FASTCALL
    #define FASTCALL
#endif

#endif
