#include<stdio.h>
#include "commanderX16.h"

char exchc;

void init_x16(void)
{
    /* 
    lda #$80
    jsr screen_set_mode

    LoadW r0, 0
    LoadW r1, 0
    LoadW r2, 0
    LoadW r3, 0
    jsr console_init
    */
    asm("lda #$80");
    asm("jsr $FF5F");
    asm("lda #0");
    asm("sta $02");
    asm("sta $03");
    asm("sta $04");
    asm("sta $05");
    asm("sta $06");
    asm("sta $07");
    asm("sta $08");
    asm("sta $09");
    console_init();
 }
void putc_x16(char c)
{
    exchc=c;    /* Register A contains the character, at this point. */
    asm("sec");
    asm("jsr $FEDE");
}

char getc_x16(void)
{
    asm("jsr $FEE1");
    asm("sta _exchc");
    return exchc;
}


void puts_x16(char *s)
{
    int i;
    for(i=0; s[i]!='\0'; ++i) putc_x16(s[i]);
}

char *gets_x16(char *buffer, unsigned int size)
{
    int i;
    char c;
    for(i=0; i<size-1; ++i) {
        c=getc_x16();
        /* I need this, as I compare the texts with strings harcoded in the
           C code and cc65 translates them into PETSCII. */
        if((c>=0x41 && c<=0x5A)||(c>=0x61 && c<=0x7A)) c^=0x20 ;
        buffer[i]=c;
        if(c==13) {
            buffer[i+1]=0;
            return buffer;
        }
    }
    return buffer;
}


#ifdef DEMOX
char buffer[256];
int main(void)
{
    init_x16();
    puts_x16("Test:\n");

    while(1) {
        gets_x16(buffer,256);
        puts_x16(buffer);
    }
    return 0;
}

#endif