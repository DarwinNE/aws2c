/*  This file is provided by the aws2c tool and it contains input/output
    routines as well as the parser. It should be compiled along with the
    generated .c file obtained by the tool. This file can be customised for
    specific needs, for example to write on a window in a GUI system instead
    of on the terminal.

    Some basic configuration can also be done by adjusting the systemdef.h file
    to your needs.

    Davide Bucci, October 2018-February 2021
*/

#include <stdio.h>
#include <string.h>
#ifndef CONFIG_FILENAME
    #include"config.h"
#else
    #include CONFIG_FILENAME
#endif
#include "systemd.h"
#include "aws.h"
#include "inout.h"

char playerInput[BUFFERSIZE];


#ifdef NROW
EFFSHORTINDEX rowc;
#endif
#ifdef NCOL
EFFSHORTINDEX colc;
#endif

unsigned int verb;
unsigned int noun1;
unsigned int noun2;
#ifndef NOADVERBS
    ADVERBTYPE adve;
#endif
ACTORTYPE actor;
#ifndef NOADJECTIVES
    ADJECTIVETYPE adjective;
#endif

extern word dictionary[];

/* The current position in the line */

EFFSHORTINDEX ls, lc;
#ifndef NCOL
EFFSHORTINDEX opc;
#endif
#ifdef DEFINEWTR
void wtr(const char *s) FASTCALL
{
    for(; *s!='\0';++s)
        fputc(*s,stdout);
}
#endif

#ifdef NCOL
void zeroc(void)
{
    colc=0;
}
#endif

/** Write a string without adding a newline. Process some codes to put in
    evidence the text and handle the word wrapping.
*/

char c,d;
#ifndef INTERNAL_DEF
    EFFSHORTINDEX pc;
    #ifdef NCOL
        char wordbuffer[NCOL];
    #else
        #define NBUF 50
        char wordbuffer[NBUF];
    #endif
#endif

void writesameln(char *line) FASTCALL
{
    #ifdef INTERNAL_DEF
    EFFSHORTINDEX pc;
    #ifdef NCOL
    char wordbuffer[NCOL];
    #else
    #define NBUF 50
    char wordbuffer[NBUF];
    #endif
    #endif
    pc=0;

    while(1){
        c=*line;
        ++line;
        d=*line;
        if(c==' ' || c=='\n' || c=='\r' ||c=='\0'
            #ifndef COMPRESSED
            || (c=='\\' && d=='n')
            #endif
            #ifndef NCOL
            || (pc==NBUF-2)
            #endif
            )
        {
            #ifndef NCOL
            if (pc==NBUF-2) {
                wordbuffer[pc]=c;
                wordbuffer[pc+1]='\0';
                opc=pc;
            } else
            #endif
                wordbuffer[pc]='\0';
            #ifdef NCOL
            if(colc>=NCOL) {
                NEWLINE_PUTC();
                colc=pc;
                #ifdef NROW
                if(++rowc>NROW) {waitkey(); zeror();};
                #endif
            }
            #endif
            PUTS(wordbuffer);
            if(c=='\0')
                return;
            pc=0;
            if(c=='\n' || c=='\r' 
                #ifndef COMPRESSED
                || (c=='\\' && d=='n')
                #endif
                ) 
            {
                #ifndef COMPRESSED
                if(c=='\\')
                    ++line;
                #endif
                NEWLINE_PUTC();
                #ifdef NCOL
                zeroc();
                #endif
                #ifdef NROW
                if(++rowc>NROW) {waitkey(); zeror();};
                #endif
            } else
            #ifdef NCOL
                if(colc<NCOL-1) 
            #endif
            {
                #ifndef NCOL
                if (opc!=NBUF-2)
                #endif
                    PUTC(' ');
                #ifdef NCOL
                ++colc;
                #endif
            }
        } else {
            wordbuffer[pc++]=c;
            #ifdef NCOL
            ++colc;
            #endif
        }
    }
}

/* Clear the screen */
void clear(void)
{
    cls();
    #ifdef NCOL
    zeroc();
    #endif
    #ifdef NROW
    zeror();
    #endif
}

#ifdef NROW
void zeror(void)
{
    rowc=0;
}
#endif

/** Same as writesameln, but adds a newline at the end of the message.
*/
void writeln(char* line) FASTCALL
{
    writesameln(line);
    NEWLINE_PUTC();
    #ifdef NROW
    if(++rowc>NROW) {waitkey(); zeror();};
    #endif
    #ifdef NCOL
    zeroc();
    #endif
}

/* 'Eat' a carriage return that may be present at the end of a string.
*/
char *eatcr(char *s) FASTCALL
{
    char *os=s;
    lc=0;
    for(;*s!='\0';++s, ++lc)
        if(*s=='\n'||*s=='\r') {
            *s='\0';
            break;
        }
    return os;
}

/** Read a line of text and return the length of the line (remove the \n char).
*/
unsigned int readln(void)
{
    inputtxt();
    writesameln("> ");
    GETS(playerInput,BUFFERSIZE);
    normaltxt();
    // remove the '\n'
    eatcr(playerInput);

    #ifdef NROW
    zeror();
    #endif
    return lc;
}


/* Having this buffer as a static variable allows to avoid running into
   troubles with Cc65 that needs to have very few local variables to
   compile to targets such as the 6502 processor. */
   
#ifndef INTERNAL_DEF
char s[BUFFERSIZE];
#endif

#ifdef DICT5BIT
/*  Handle 5-bit for character compressed dictionary.
    The result is a null-terminated string that substitutes the original one.
*/
void compress_5bit(char *buffer)
{
    char *pcomp=buffer;
    unsigned int shift=0;
    unsigned int c;

    while((c=*buffer)!='\0') {
        *(buffer++)='\0';
        //c=toupper(c);  // Is not needed here, as already uppercase.
        c=(c-'@')&0x1F;  // 'A' is encoded with 1, since 0 is the end of string
        c<<=shift;
        *pcomp |=c&0x00FF;
        if(shift>=3)
            *(++pcomp)=(c&0xFF00)>>8;
        shift=(shift+5)&0x07;
    }
}
#endif

#ifdef DICTHASH
/*  Store the dictionary as a 3-byte hash code for each word.
    The result is a 3-character string that substitutes the original one.
*/
void compress_hash(char *buffer)
{
    unsigned char c;
    unsigned int i=0, j=0;
    #define N 3
    while((c=buffer[j++])!='\0') {
        #ifdef COMMODORE8BIT
        if(c>=0xc1 && c<=0xdA) c^=0x80;
        buffer[j-1]=c;
        #endif
        if(j>N) {
            c^=(0xFF -i);
            buffer[i]^=c;
        }
        if(++i>N-1) i=0;
    }
    if(j<2) buffer[1]=0;
    if(j<3) buffer[2]=0;
}
#endif

/** Main parser.
*/
void interrogationAndAnalysis()
{
    #ifdef INTERNAL_DEF
    char s[BUFFERSIZE];
    #endif
    unsigned int i, k;
    char c;
    word* te;

    if(ls==0) {
        readln();
    }
    verb=0;
    noun1=0;
    noun2=0;
    #ifndef NOADVERBS
        adve=0;
    #endif
    actor=0;
    #ifndef NOADJECTIVES
        adjective=0;
    #endif

    while(ls<lc) {
        for(k=0; ls<lc && k<BUFFERSIZE; ++ls) {
            c=playerInput[ls];
            if(c==' ') {
                ++ls;
                break;
            }

            if(c>='a' && c<='z')
                c-='a'-'A';  // Convert to uppercase

            s[k++]=c;
        }
        s[k]='\0';
        #ifdef DICT5BIT
        compress_5bit(s);
        #endif
        #ifdef DICTHASH
        compress_hash(s);
        #endif
        // s now contains the word. Search to find
        // if it is recognized or not.
        for(i=0;i<DSIZE; ++i) {
            te=&dictionary[i];
            #ifdef DICTHASH
            if( s[0]==te->c1&&
                s[1]==te->c2&&
                s[2]==te->c3)
            {
            #else
            if(strcmp(s,te->w)==0) {
            #endif
                k=te->code;
                switch (te->t) {
                    case VERB:
                        verb=k;
                        break;
                    case NAME:
                        if(noun1==0) {
                            noun1=k;
                        } else {
                            noun2=k;
                        }
                        break;
                    #ifndef NOADVERBS
                        case ADVERB:
                            adve=k;
                            break;
                    #endif
                    case ACTOR:
                        actor=k;
                        break;
                    #ifndef NOADJECTIVES
                        case ADJECTIVE:
                            adjective=k;
                            break;
                    #endif
                    case SEPARATOR:
                        /*  The line is not finished but it can be executed. */
                        return;
                    default:
                        break;
                }
                break;
            }
        }
    }
    /*  The scanning has finished because the line is complete. Read a new line
        the next time the function is called */
    ls=0;
    //printf("verbo=%d, nome1=%d, nome2=%d\n",verb,noun1,noun2);
}
