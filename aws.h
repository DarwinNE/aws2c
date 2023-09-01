#ifndef _AWS_H_
#define _AWS_H_

#include <stdio.h>
typedef unsigned char boolean;
#define true 1
#define false 0

typedef unsigned char word_type;
#define ADVERB 0
#define VERB 1
#define NAME 2
#define SEPARATOR 3
#define ACTOR 4
#define ADJECTIVE 5


typedef struct info_d {
    char *version;
    unsigned int textcolor;
    unsigned int backcolor;
    unsigned int textcolordark;
    unsigned int backcolordark;
    char *name;
    char *author;
    char *date;
    char *description;
    unsigned int code;
    char *fontname;
    unsigned int charsize;
    unsigned int fontstyle;
    unsigned int startroom;
    boolean graphical;
    unsigned int maxcarryingw;
    unsigned int maxcarryings;
} info;

typedef struct word_d {
#ifdef DICTHASH
    char c1,c2,c3;
#else
    char *w;
#endif
    unsigned int code;
    word_type t;
} word;

#ifndef BYTEROOMCODE
    typedef unsigned int room_code;
#else
    typedef unsigned char room_code;
#endif

typedef struct room_d {
    room_code code;
    const char *long_d;
    #ifndef AVOID_SDESC
    const char *s;
    #endif
    const char *short_d;

    #ifndef DIR_REDUCED
        #define NDIR 10
        // north, sud, east, west, up, down, north east, north west, south east,
        // south west
        room_code directions[NDIR];
    #else
        #define NDIR 6
        // north, sud, east, west, up, down
        room_code directions[NDIR];
    #endif
} room;

typedef struct message_d {
    unsigned int code;
    char *txt;
} message;

#ifndef BYTEOBJCODE
    typedef unsigned int obj_code;
#else
    typedef unsigned char obj_code;
#endif

#define ISNOTMOVABLE  1
#define ISWEREABLE    2
        
typedef struct object_d {
    obj_code code;
    #ifndef NOLONGDESC
        char *s;
    #endif
    const char *desc;
    #ifndef NOSIZEWEIGHT    // Don't consider size and weight.
        unsigned int weight;
        unsigned int size;
    #endif
    unsigned int position;      // Always int, as carried =1500, worn=1600
    unsigned char attributes;
} object;

typedef struct tree_d {
    unsigned char c;
    unsigned char son0idx;
    unsigned char son1idx;
} tree;

void restart(void);

#ifdef ATARI_ST
#define NEWLINE "\r\n"
#define NEWLINE_PUTC() PUTC('\r'); PUTC('\n');
#else
#define NEWLINE "\n"
#define NEWLINE_PUTC() PUTC('\n');
#endif

#endif
