/*  This file is provided by the aws2c tool and it contains input/output
    routines as well as the parser. It should be compiled along with the
    generated .c file obtained by the tool. This file can be customised for
    specific needs, for example to write on a window in a GUI system instead
    of on the terminal.

    Some basic configuration can also be done by adjusting the systemdef.h file
    to your needs.

    Davide Bucci, October 2018-January 2019
*/

#include <stdio.h>
#include <string.h>
#include "systemd.h"
#include "aws.h"
#include "inout.h"

char playerInput[BUFFERSIZE];
char wordbuffer[NCOL];
EFFSHORTINDEX colc;

#ifdef NROW
EFFSHORTINDEX rowc;
#endif

unsigned int verb;
unsigned int noun1;
unsigned int noun2;
unsigned int adve;
unsigned int actor;
unsigned int adjective;

extern word dictionary[];

/* The current position in the line */

EFFSHORTINDEX ls, lc;
EFFSHORTINDEX pc;

#ifdef DEFINEWTR
void wtr(const char *s) FASTCALL
{
    int i;
    for(i=0; s[i]!='\0';++i)
        fputc(s[i],stdout);
}
#endif


/** Write a string without adding a newline. Process some codes to put in
    evidence the text and handle the word wrapping.
*/
void writesameln(char *line) FASTCALL
{
    char c,d;
    pc=0;

    while(1){
        c=*line;
        ++line;
        d=*line;
        if(c==' ' || c=='\n' || c=='\r' ||c=='\0'
            #ifndef COMPRESSED
            || (c=='\\' && d=='n')
            #endif
            ) {
            wordbuffer[pc]='\0';
            if(colc>=NCOL) {
                PUTC('\n');
                colc=pc;
                #ifdef NROW
                if(++rowc>NROW) {waitkey(); rowc=0;};
                #endif
            }
            PUTS(wordbuffer);
            if(c=='\0')
                return;
            pc=0;
            if(c=='\n' || c=='\r' 
            #ifndef COMPRESSED
            || (c=='\\' && d=='n')
            #endif
            ) {
                #ifndef COMPRESSED
                if(c=='\\')
                    ++line;
                #endif
                PUTC('\n');
                colc=0;
                #ifdef NROW
                if(++rowc>NROW) {waitkey(); rowc=0;};
                #endif
            } else if(colc<NCOL-1) {
                PUTC(' ');
                ++colc;
            }
        } else {
            wordbuffer[pc++]=c;
            ++colc;
        }
    }
}

/* Clear the screen */
void clear(void)
{
    cls();
    colc=0;
    #ifdef NROW
    rowc=0;
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
    PUTC('\n');
    #ifdef NROW
    if(++rowc>NROW) {waitkey(); rowc=0;};
    #endif
    colc=0;
}

/** Read a line of text and return the length of the line (remove the \n char).
*/
unsigned int readln(void)
{
    unsigned int lc;
    inputtxt();
    writesameln("> ");
    GETS(playerInput,BUFFERSIZE);
    normaltxt();
    lc=strlen(playerInput);
    // remove the '\n'
    if(lc>0) {
        playerInput[lc-1]='\0';
        --lc;
    }
    #ifdef NROW
    rowc=0;
    #endif
    return lc;
}


/* Having this buffer as a static variable allows to avoid running into
   troubles with Cc65 that needs to have very few local variables to
   compile to targets such as the 6502 processor. */
   
char s[BUFFERSIZE];



/** Main parser.
*/
void interrogationAndAnalysis(unsigned int num_of_words) FASTCALL
{
    unsigned int i, k;
    char c;

    if(ls==0) {
        lc=readln();
    }
    verb=0;
    noun1=0;
    noun2=0;
    adve=0;
    actor=0;
    adjective=0;

    while(ls<lc) {
        for(k=0; ls<lc && k<BUFFERSIZE; ++ls) {
            c=playerInput[ls];
            if(c==' ' || c=='\'') {
                ++ls;
                break;
            }
            if(c>='a' && c<='z')
                c-='a'-'A';  // Convert to uppercase

            s[k++]=c;
        }
        s[k]='\0';
        // s now contains the word. Search to find
        // if it is recognized or not.
        for(i=0;i<num_of_words; ++i) {
            if(strcmp(s,dictionary[i].w)==0) {
                k=dictionary[i].code;
                switch (dictionary[i].t) {
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
                    case ADVERB:
                        adve=k;
                        break;
                    case ACTOR:
                        actor=k;
                        break;
                    case ADJECTIVE:
                        adjective=k;
                        break;
                    case SEPARATOR:
                        /*  The line is not finished but it can be executed. */
                        return;
                    default:
                        break;
                }
            }
        }
    }
    /*  The scanning has finished because the line is complete. Read a new line
        the next time the function is called */
    ls=0;
}
