#ifndef _AWS_H_
#define _AWS_H_

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
    char *w;
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
    char *long_d;
    #ifndef AVOID_SDESC
    char *s;
    #endif
    char *short_d;

    #ifndef DIR_REDUCED
        // north, sud, east, west, up, down, north east, north west, south east,
        // south west
        room_code directions[10];
    #else
        // north, sud, east, west, up, down
        room_code directions[6];
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

typedef struct object_d {
    obj_code code;
    #ifndef NOLONGDESC
        char *s;
    #endif
    char *desc;
    #ifndef NOSIZEWEIGHT    // Don't consider size and weight.
        unsigned int weight;
        unsigned int size;
    #endif
    unsigned int position;      // Always int, as carried =1500, weared=1600
    boolean isnotmovable;
    boolean isnotwereable;
} object;

typedef struct tree_d {
    unsigned char c;
    unsigned char son0idx;
    unsigned char son1idx;
} tree;

#endif