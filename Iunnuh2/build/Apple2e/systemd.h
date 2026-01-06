#ifndef __SYSTEMDEF_H__
#define __SYSTEMDEF_H__

    #include<stdio.h>
    #include<apple2enh.h>

    #define BUFFERSIZE 40
    #define B_SIZE 20
    #define FASTCALL 

    #define waitscreen()
    #define LOAD 
    #define SAVE 
    // The number of columns of the screen
    #ifndef NCOL
        #define NCOL 40
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
    #define tab()
    #define wait1s()    {}

    #define init_term() {}
    #define leave()

#ifndef EFFSHORTINDEX
    #define EFFSHORTINDEX unsigned char
#endif

#ifdef NROW
    extern EFFSHORTINDEX rowc;
#endif

#ifndef SHIFTPETSCII
    #define SHIFTPETSCII
#endif

#ifndef end_inputtxt
    #define end_inputtxt()
#endif

#ifndef end_evidence1
    #define end_evidence1()
#endif

#ifndef end_evidence2
    #define end_evidence2()
#endif

#ifndef end_evidence3
    #define end_evidence3()
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
