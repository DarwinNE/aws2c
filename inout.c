#include <stdio.h>
#include <string.h>
#include "modern.h"
#include "aws.h"

#define NCOL 80
#define BUFFERSIZE 256

char playerInput[BUFFERSIZE];
char wordbuffer[NCOL*2];
int colc;


int verb;
int noun1;
int noun2;
int adve;

extern word dictionary[];


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

void writeln(char* line)
{
    writesameln(line);
    printf("\n");
    colc=0;
}

/** read a line of text and return the length of the line (remove the \n char).
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


void interrogationAndAnalysis(int num_of_words)
{
    boolean search=true;
    char s[BUFFERSIZE];
    int ls=0, lc, i, x, ols, app,k;
    boolean found = false;
    lc=readln();
    verb=0;
    noun1=0;
    noun2=0;
    adve=0;
    char c;

    while(ls<lc) {
        k=0;
        for(; ls<lc; ++ls) {
            c=playerInput[ls];
            if(c==' ') {
                ls++;
                break;
            }
            if(c>='a' && c<='z')
                c-=32;  // Convert to uppercase
            s[k++]=c;
        }
        s[k]='\0';
        
        // s now contains the word. Search to find
        // if it is recognized or not.
        for(i=0;i<num_of_words; ++i) {
            if(strcmp(s,dictionary[i].w)==0) {
        /*        printf("type %d, word %s, code %d\n",
                    dictionary[i].t, dictionary[i].w, dictionary[i].code); */
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