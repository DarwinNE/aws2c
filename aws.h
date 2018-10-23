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

typedef struct room_d {
    unsigned int code;
    char *long_d;
    char *s;
    char *short_d;

    #ifndef DIR_REDUCED
    // north, sud, east, west, up, down, north east, north west, south east,
    // south west
    unsigned int directions[10];
    #else
    // north, sud, east, west, up, down
    unsigned int directions[6];
    #endif
} room;

typedef struct message_d {
    unsigned int code;
    char *txt;
} message;

typedef struct object_d {
    unsigned int code;
    char *s;
    char *desc;
    unsigned int weight;
    unsigned int size;
    unsigned int position;
    boolean isnotmovable;
    boolean isnotwereable;
} object;

typedef struct tree_d {
    unsigned char c;
    unsigned char son0idx;
    unsigned char son1idx;
} tree;

#endif