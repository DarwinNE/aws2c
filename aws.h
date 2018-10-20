#ifndef _AWS_H_
#define _AWS_H_

typedef enum boolean_d {true=-1, false=0} boolean;

typedef unsigned char word_type;
#define ADVERB 0
#define VERB 1
#define NAME 2
#define SEPARATOR 3


typedef struct info_d {
    char *version;
    int textcolor;
    int backcolor;
    int textcolordark;
    int backcolordark;
    char *name;
    char *author;
    char *date;
    char *description;
    int code;
    char *fontname;
    int charsize;
    int fontstyle;
    int startroom;
    boolean graphical;
    int maxcarryingw;
    int maxcarryings;
} info;

typedef struct word_d {
    char *w;
    int code;
    word_type t;
} word;

typedef struct room_d {
    int code;
    char *long_d;
    char *s;
    char *short_d;

    #ifndef DIR_REDUCED
    // north, sud, east, west, up, down, north east, north west, south east,
    // south west
    int directions[10];
    #else
    // north, sud, east, west, up, down
    int directions[6];
    #endif
} room;

typedef struct message_d {
    int code;
    char *txt;
} message;

typedef struct object_d {
    int code;
    char *s;
    char *desc;
    int weight;
    int size;
    int position;
    boolean isnotmovable;
    boolean isnotwereable;
} object;

typedef struct tree_d {
    int c;
    unsigned char son0idx;
    unsigned char son1idx;
} tree;

#endif