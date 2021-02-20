/*  This file is provided by the aws2c tool and it contains code needed for
    loading and saving the current state of the game.

    Davide Bucci, December 2019-September 2020
*/

#include<stdio.h>
#include<string.h>
#ifndef CONFIG_FILENAME
    #include"config.h"
#else
    #include CONFIG_FILENAME
#endif
#include"systemd.h"
#include"aws.h"
#include"loadsave.h"
#include"inout.h"

extern room_code current_position;
extern room_code next_position;
extern boolean marker[];
extern int counter[];
extern object obj[];
extern room world[];

extern char playerInput[];

#if OSIZE<256 && RSIZE<256
    unsigned char i;
#else
    unsigned int i;
#endif
unsigned char j;

FILE *f;

/*  I know there is atoi, but I tried it and my barebone implementation
    allows to spare a good 120 bytes on the 6502 with Cc65.
*/
int s2i(char *s)
{
    int sign=1, val=0;

    if(*s=='-') {
        sign=-1;
        ++s;
    }
    while(*s!='\0') {
        if(*s<'0' || *s>'9')
            break;
        val=val*10+(*s++-'0');
    }
    return val*sign;
}

char *i2s(char *s, int v)
{
    unsigned char r,i=0,j=0;

    if(v<0) {
        s[0]='-';
        v=-v;
        j=i=1;
    }
    do {
        r=v%10;
        s[i++]='0'+r;
        v=v/10;
    } while(v>0);
    s[i--]='\0';
    while(i>j) {
        r=s[i];
        s[i]=s[j];
        s[j]=r;
        ++j;
        --i;
    }
    return s;
}

void wri(int v)
{
    fputs(i2s(playerInput,v),f);
    fputs("\n",f);
}

/*  Save the current game.
    Possible outcomes
    - Everything was OK.
    - Could not open file.
*/
void savegame(char *filename) FASTCALL
{
    f=fopen(eatcr(filename),"w");

    if(f==NULL) {
        PUTS("Error: can not open file ");
        return;
    }
    fputs("SAVEDAWS2.1\n",f);

    fputs(GAMEN"\n",f);
    wri((int)current_position);

    for(i=0;i<129;++i) {
        wri((int)counter[i]);
    }

    for(i=0;i<129;++i) {
        wri((int)marker[i]);
    }

    for(i=0;i<OSIZE;++i) {
        wri((int)obj[i].position);
    }

    for(i=0;i<RSIZE;++i) {
        for(j=0;j<NDIR;++j)
            wri((int)world[i].directions[j]);
    }

    fclose(f);
    return;
}

int rei(void) FASTCALL
{
    fgets(playerInput, BUFFERSIZE, f);
    return s2i(playerInput);
}

/*  Load a game.
    Possible errors
    - Could not open input file.
    - Incorrect format.
    - Can't read file contents.
    - Wrong game.
*/
void loadgame(char *filename) FASTCALL
{
    f=fopen(eatcr(filename),"r");
    if(f==NULL) {
        PUTS("Can not open file ");
        return;
    }

    fgets(playerInput, BUFFERSIZE, f);
    if(strcmp(playerInput, "SAVEDAWS2.1\n")!=0) {
        PUTS("Incorrect format ");
        fclose(f);
        return;
    }
    fgets(playerInput, BUFFERSIZE, f);
    if(strcmp(playerInput, GAMEN"\n")!=0) {
        PUTS("Incorrect game: ");
        PUTS(playerInput);
        fclose(f);
        return;
    }

    next_position=(room_code) rei();

    for(i=0;i<129;++i)
        counter[i]=rei();

    for(i=0;i<129;++i)
        marker[i]=rei();

    for(i=0;i<OSIZE;++i)
        obj[i].position=rei();

    for(i=0;i<RSIZE;++i)
        for(j=0;j<NDIR;++j) 
            world[i].directions[j]=(room_code) rei();

    fclose(f);
    marker[120]=false;   /* Describe again the current location */
    return;
}