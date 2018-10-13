/*  This file is provided by the aws2c tool and it contains input/output
    routines as well as the parser. It should be compiled along with the
    generated .c file obtained by the tool. This file can be customised for
    specific needs, for example to write on a window in a GUI system instead
    of on the terminal.

    Some basic configuration can also be done by adjusting the systemdef.h file
    to your needs.

    Davide Bucci, October 2018
*/

#include <stdio.h>
#include <string.h>
#include "systemdef.h"
#include "aws.h"

char playerInput[BUFFERSIZE];
char wordbuffer[NCOL*2];
int colc;


int verb;
int noun1;
int noun2;
int adve;

extern word dictionary[];

/** Write a string without adding a newline. Process some codes to put in
    evidence the text and handle the word wrapping.
*/
void writesameln(char *line)
{
    int i=0;
    boolean flag=false;
    boolean norm=false;
    char c;
    int pc=0;

    while(1){
        c=line[i++];
        if(c=='*' && flag==false) {
            evidence2();
            flag=true;
        } else if(c=='*' && flag==true) {
            norm=true;
            flag=false;
        } else if(c=='-' && flag==false) {
            evidence1();
            flag=true;
        } else if(c=='-' && flag==true) {
            norm=true;
            flag=false;
        } else if(c=='\b') {
            waitscreen();
        } else if(c=='\t') {
            tab();
        } else if(c==' ' || c=='\n'||c=='\0') {
            wordbuffer[pc]='\0';
            if(colc>=NCOL) {
                printf("\n");
                colc=strlen(wordbuffer);
            }
            printf("%s",wordbuffer);
            if(norm==true) {
                normaltxt();
                norm=false;
            }
            if(c=='\0')
                return;
            pc=0;
            if(c=='\n') {
                printf("\n");
                colc=0;
            } else if(colc<NCOL-1) {
                printf(" ");
                ++colc;
            }
        } else {
            wordbuffer[pc++]=c;
            ++colc;
        }
    }
}

/** Same as writesameln, but adds a newline at the end of the message.
*/
void writeln(char* line)
{
    writesameln(line);
    printf("\n");
    colc=0;
}

/** Read a line of text and return the length of the line (remove the \n char).
*/
int readln(void)
{
    int lc;
    inputtxt();
    writesameln("> ");
    fgets(playerInput,BUFFERSIZE,stdin);
    normaltxt();
    lc=strlen(playerInput);
    // remove the '\n'
    if(lc>0) {
        playerInput[lc-1]='\0';
        --lc;
    }
    return lc;
}


// Having this buffer as a static variable allows to avoid running into
// troubles with Cc65 that needs to have very few local variables to
// compile to targets such as the 6502 processor.
char s[BUFFERSIZE];

/** Main parser.
*/
void interrogationAndAnalysis(int num_of_words)
{
    boolean search=true;
    int ls=0, lc, i, k;
    boolean found = false;
    char c;

    lc=readln();
    verb=0;
    noun1=0;
    noun2=0;
    adve=0;

    while(ls<lc) {
        k=0;
        for(; ls<lc && k<BUFFERSIZE; ++ls) {
            c=playerInput[ls];
            if(c==' ' || c=='\'') {
                ls++;
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
                switch (dictionary[i].t) {
                    case VERB:
                        verb=dictionary[i].code;
                        break;
                    case NAME:
                        if(noun1==0) {
                            noun1=dictionary[i].code;
                        } else {
                            noun2=dictionary[i].code;
                        }
                        break;
                    case ADVERB:
                        adve=dictionary[i].code;
                        break;
                    case SEPARATOR:
                        break;
                    default:
                        break;
                }
            }
        }
        fflush(stdout);
    }
}