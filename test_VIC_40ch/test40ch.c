#include "vic40col.h"

#define BUFSIZE 38

int main(void)
{
    char buffer[BUFSIZE];
    initGraphic();
    negative();
    puts40ch("Please enter your name: \n");
    positive();
    puts40ch("> ");
    gets40ch(buffer, BUFSIZE-1);
    puts40ch("Hello ");
    puts40ch(buffer);
    puts40ch("\n\n");
    cgetc40ch();
    normalText();
    return 0;
}