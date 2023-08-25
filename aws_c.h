#ifndef _AWS_C_H_
#define _AWS_C_H_

// This file is used by aws2c.c but not by its output files.

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

typedef unsigned int room_code;

typedef struct room_d {
    room_code code;
    char *long_d;
    char *s;
    char *short_d;

    #define NDIR 10
    // north, sud, east, west, up, down, north east, north west, south east,
    // south west
    room_code directions[NDIR];
} room;

typedef struct message_d {
    unsigned int code;
    char *txt;
} message;

typedef unsigned int obj_code;

#define ISNOTMOVABLE  1
#define ISWEREABLE    2
        
typedef struct object_d {
    obj_code code;
    char *s;
    char *desc;
    unsigned int weight;
    unsigned int size;
    unsigned int position;      // Always int, as carried =1500, weared=1600
    unsigned char attributes;
} object;

typedef struct tree_d {
    unsigned char c;
    unsigned char son0idx;
    unsigned char son1idx;
} tree;

void restart(void);

#endif