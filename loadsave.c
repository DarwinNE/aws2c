/*  This file is provided by the aws2c tool and it contains code needed for
    loading and saving the current state of the game.

    Davide Bucci, December 2019
*/

#include<stdio.h>
#include<string.h>
#include"config.h"
#include"systemd.h"
#include"aws.h"
#include"loadsave.h"

extern room_code current_position;
extern room_code next_position;
extern boolean marker[];
extern int counter[];
extern object obj[];
extern room world[];

#define BUFSIZE 40
char buffer[BUFSIZE+1];

int s2i(char *s)
{
    int i=0;
    int sign=1;
    int val=0;

    if(s[i]=='-') {
        sign=-1;
        ++i;
    }
    while(s[i]!='\0') {
        if(s[i]<'0' || s[i]>'9')
            break;
        val=val*10+(s[i++]-'0');
    }
    return val*sign;
}

char *i2s(char *s, int v)
{
    int i=0,j=0;
    int m=0;
    int r;

    if(v<0) {
        s[i++]='-';
        v=-v;
        j=1;
    } else if(v==0) {
        s[i++]='0';
        s[i]='\0';
        return s;
    }
    while(v>0) {
        r=v%10;
        v=v/10;
        s[i++]='0'+r;
        ++m;
    }
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

/* 'Eat' a carriage return that may be present at the end of a string.
*/
char *eatcr(char *s)
{
    int i;
    for(i=0;s[i]!='\0';++i)
        if(s[i]=='\n'||s[i]=='\r') {
            s[i]='\0';
            break;
        }
    return s;
}

/*  Save the current game.
    rsize: number of rooms in the game.
    osize: number of objects in the game.
    Return value:
    0 - Everything was OK.
    1 - Could not open file.
*/
int savegame(char *filename, int rsize, int osize)
{
    unsigned int i,j,t;
    FILE *f;

    f=fopen(eatcr(filename),"w");
    if(f==NULL)
        return 1;
    fputs("SAVEDAWS2.1\n",f);
    fputs(i2s(buffer,(int)current_position),f);
    fputs("\n",f);

    for(i=0;i<129;++i) {
        fputs(i2s(buffer,(int)counter[i]),f);
        fputs("\n",f);
    }

    for(i=0;i<129;++i) {
        t=(int)marker[i];
        fputs(i2s(buffer,t),f);
        fputs("\n",f);
    }

    for(i=0;i<osize;++i) {
        t=(int)obj[i].position;
        fputs(i2s(buffer,t),f);
        fputs("\n",f);
    }
    for(i=0;i<rsize;++i)
        for(j=0;j<NDIR;++j) {
            t=(int)world[i].directions[j];
            fputs(i2s(buffer,t),f);
            fputs("\n",f);
        }
    
    fclose(f);
    return 0;
}

/*  Load a game.
    rsize: number of rooms in the game.
    osize: number of objects in the game.
    Return value:
    0 - Everything was OK.
    1 - Could not open input file.
    2 - Incorrect format.
    3 - Can't read file contents.
*/
int loadgame(char *filename, int rsize, int osize)
{
    int i,j,t;
    FILE *f;

    f=fopen(eatcr(filename),"r");
    if(f==NULL)
        return 1;

    fgets(buffer, BUFSIZE, f);
    if(strcmp(buffer, "SAVEDAWS2.1\n")!=0) {
        fclose(f);
        return 2;
    }

    fgets(buffer, BUFSIZE, f);
    t=s2i(buffer);
    next_position=(room_code) t;

    for(i=0;i<129;++i) {
        fgets(buffer, BUFSIZE, f);
        counter[i]=s2i(buffer);
    }

    for(i=0;i<129;++i) {
        fgets(buffer, BUFSIZE, f);
        marker[i]=(boolean)s2i(buffer);
    }

    for(i=0;i<osize;++i) {
        fgets(buffer, BUFSIZE, f);
        obj[i].position=s2i(buffer);
    }
    
    for(i=0;i<rsize;++i)
        for(j=0;j<NDIR;++j) {
            fgets(buffer, BUFSIZE, f);
            world[i].directions[j]=(room_code) s2i(buffer);
        }
    
    fclose(f);
    marker[120]=false;   /* Describe again the current location */
    return 0;
}