/*

    AWS to C converter by Davide Bucci

AWS stands for Adventure Writing System and is a program developed by
Aristide Torrelli to write interactive fiction games. Here is a link to his
blog, in Italian language:

http://www.aristidetorrelli.it/aws3/AWS.html

The game developed with AWS is described by a compact file that contains the
vocabulary, the messages, the descriptions and all the logic needed for the
game.

Its structure of AWS is relatively simple yet powerful and I decided to write
this little converter that automagically generates C code that implements the
logic described by the game.

If you need an English description of AWS, I wrote this:

https://github.com/DarwinNE/aws2c/blob/master/AWS_description.md

The parser and some input/output functions are contained in an external file
called inout.c that can be customised to the target machine.

I tested this system with gcc and the generated files can be compiled with
any reasonably standard C compiler. I tested them with cc65 to target
old Commodore machines.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "aws_c.h"
#include "compress.h"

#define VERSION "1.9.6, September 2018 - April 2021"
#define AREYOUSURE "Are you sure? Type 'Y' and return if yes."
#define EXITRESTART "'E' and return to exit, anything else to restart."

/* TO DO

- Finish implementing the remaining actions, functions and decisions.
- Free the allocated memory before quitting the program.
- Test, test!

*/

#define BUFFERSIZE 16000
char buffer[BUFFERSIZE];
char function_res[BUFFERSIZE];
char *config_file=NULL;
#define TAB "    "

room* world;
unsigned int rsize;


#define start_function() function_res[0]='\0';

typedef struct localc_t {
    unsigned int room;
    char *condition;
} localc;

unsigned int no_of_errors;
boolean verbose=false;
boolean convert_utf8=false;
boolean convert_accents=false;
boolean convert_accent_alt=false;
boolean compress_messages=false;
boolean compress_descriptions=false;
boolean use_6_directions=false;
boolean shortcuts=true;
boolean hardcoded_messages=false;
boolean add_clrscr=true;
boolean dont_care_size_weight=false;
boolean no_obj_long_desc=true;
boolean strip_empty_messages=false;
boolean no_header=false;
boolean strip_automatic_counters=false;
boolean compress5bit_dict=false;
boolean dont_use_light=false;
boolean compress_hash_dict=false;

boolean actor_as_byte=true;
boolean adjective_as_byte=true;
boolean adverb_as_byte=true;
boolean use_adverbs=false;
boolean use_adjectives=false;

boolean complete_shortcut=false;
boolean checked_noun1_greater_zero=false;
boolean dont_issue_message=false;
boolean no_header_description=false;


/* Some functions are included in the code only if necessary. Those flags
   take care of them so they can be included in the code if they have been
   used.
*/
boolean need_searchw=false;
boolean need_vov=false;
boolean need_vovn=false;
boolean need_non1=false;
boolean need_cvn=false;
boolean need_check_verb_actor=false;
boolean need_cv=false;
boolean need_hold=false;
boolean need_cvna=false;
boolean need_sendallroom=false;
boolean need_unwear=false;
boolean need_iscarrsome=false;
boolean need_iswearsome=false;
boolean need_checkexit=true;
boolean need_amsm=false;
boolean need_as=false;
boolean need_ar=false;
boolean need_ams=false;
boolean need_cpi=false;
boolean need_ok=false;

/*  Statistics for optimizations.
*/
unsigned int number_of_jumps;
unsigned int number_of_drops;

/*  Those are flags than in principle can be turned on and off to optimize if
    a particular function should be defined check_position_marker_on a macro instead. This is useful
    check_position_marker_on if it is called only once, the overhead of the function calling
    mechanism can do more harm than good. Aws2c will determine
    alone if those options must be turned on or off by checking the statistics.
*/
boolean jump_as_function=false;
boolean drop_as_function=true;  // It must be true check_position_marker_on function returns a value.

typedef struct conv_t {
    char *orig;
    char *conv;
    char accent;
    char accent_alt;
} conv;

/* This is a minimal conversion that should work for the Italian language
   (at least). It is used to the conversion between UTF-8 chars and standard
   ASCII characters, plus the accents. */

#define CONVSIZE 26

char apostrophe[]={0xE2, 0x80, 0x99,0x0};
char ellips[]={0xE2, 0x80, 0xA6,0x0};
char quotel[]={0xE2, 0x80, 0x9C,0x0};
char quoter[]={0xE2, 0x80, 0x9D,0x0};


conv conversion[CONVSIZE] = {
    {"à","a",'`','\''},
    {"è","e",'`','\''},
    {"ì","i",'`','\''},
    {"ò","o",'`','\''},
    {"ù","u",'`','\''},
    {"á","a",'\'','\''},
    {"é","e",'\'','\''},
    {"í","i",'\'','\''},
    {"ó","o",'\'','\''},
    {"ô","o",'\0','\0'},
    {"ú","u",'\'','\''},

    {"À","A",'`','\''},
    {"È","E",'`','\''},
    {"Ì","I",'`','\''},
    {"Ò","O",'`','\''},
    {"Ù","U",'`','\''},
    {"Á","A",'\'','\''},
    {"É","E",'\'','\''},
    {"Í","I",'\'','\''},
    {"Ó","O",'\'','\''},
    {"Ú","U",'\'','\''},

    {"’","\'",'\0','\0'},
    {apostrophe,"\'",'\0','\0'},
    {quotel,"\'",'\0','\0'},
    {quoter,"\'",'\0','\0'},
    {ellips,"...",'\0','\0'}
};

/** Change and encode characters that may create troubles when output, such
    check_position_marker_on ".
    Exploits buffer.
*/
char *encodechar(char *input)
{
    unsigned int i,j,k=0,t;
    char c,v;
    boolean byte3, found;
    char notshown[4];
    byte3=false;

    for(i=0; (c=input[i])!='\0' && i<BUFFERSIZE-1;++i) {
        if((c=='\"')&&(compress_messages==false)) {
            buffer[k++]='\\';
        } else if(c==-25) {     // 0xE7
            c=','; // Comma is translated in AWS files!
        } else if(c=='^' && input[i+1]=='M') {
            if(compress_messages==false) {
                buffer[k++]='\\';
                c='n';
            } else {
                c='\n';
            }
            ++i;
        } else if(c&'\x80' && convert_utf8==true) {
            for(j=0; j<CONVSIZE;++j) {
                found=true;
                for(t=0;(v=conversion[j].orig[t])!='\0';++t) {
                    if(input[i+t]!=v) {
                        found=false;
                        break;
                    }
                }
                if(found==true){
                    if(convert_accents==true) {
                        for(t=0;(v=conversion[j].conv[t])!='\0';++t)
                            buffer[k++]=v;
                        if(conversion[j].accent!='\0') {
                            if(convert_accent_alt==true)
                                buffer[k++]=conversion[j].accent_alt;
                            else
                                buffer[k++]=conversion[j].accent;
                        }
                    } else {
                        for(t=0;(v=conversion[j].conv[t])!='\0';++t)
                            buffer[k++]=v;
                    }
                    i+=strlen(conversion[j].orig)-1;
                    goto cont;
                }
            }
            if(j==CONVSIZE) {
                if(byte3==false) {
                    notshown[0]=c;
                    notshown[1]=input[i+1];
                    notshown[2]='\0';
                    fprintf(stderr,
                        "WARNING: UTF-8 character %s (0x%X 0x%X)"
                        " has not been converted in \"%s\".\n",
                        notshown, (unsigned int) c, (unsigned int) input[i+1],
                        input);
                } else {
                    notshown[0]='\x80';
                    notshown[1]=c;
                    notshown[2]=input[i+1];
                    notshown[3]='\0';
                    fprintf(stderr,
                        "WARNING: UTF-8 character %s (0x80 0x%X 0x%X)"
                        " has not been converted in \"%s\".\n",
                        notshown, (unsigned int) c, (unsigned int) input[i+1],
                        input);
                }
            }
        }
        buffer[k++]=c;
cont:   t++;
    }
    buffer[k++]='\0';
    return buffer;
}

/** Read the dictionary contained in the file. The number of words to be read
    should have been already found.
    @return a pointer to the allocated dictionary, or NULL if something bad
        happened.
*/
word *read_dictionary(FILE *f, int size)
{
    unsigned int cw;
    fpos_t pos;
    char *errorp="Could not allocate enough memory for the dictionary.\n";
    if (size<=0)
        return NULL;

    word *dictionary = (word*)calloc(size, sizeof(word));
    if (dictionary==NULL) {
        printf("%s",errorp);
        return NULL;
    }
    for(cw=0; cw<size;++cw) {
        fscanf(f,"%80s", buffer);
        dictionary[cw].w=malloc((strlen(buffer)+1)*sizeof(char));
        if (dictionary[cw].w==NULL) {
            free(dictionary);
            printf("%s",errorp);
            return NULL;
        }
        strcpy(dictionary[cw].w,buffer);
        fscanf(f,"%d", &dictionary[cw].code);
        fscanf(f,"%80s", buffer);
        if(strcmp(buffer, "VERBO")==0) {
            dictionary[cw].t=VERB;
        } else if(strcmp(buffer, "AVVERBIO")==0) {
            use_adverbs=true;
            dictionary[cw].t=ADVERB;
            if(dictionary[cw].code>255)
                adverb_as_byte=false;
        } else if(strcmp(buffer, "SEPARATORE")==0) {
            dictionary[cw].t=SEPARATOR;
        } else if(strcmp(buffer, "NOME")==0) {
            dictionary[cw].t=NAME;
        } else if(strcmp(buffer, "ATTORE")==0) {
            dictionary[cw].t=ACTOR;
            if(dictionary[cw].code>255)
                actor_as_byte=false;
        } else if(strcmp(buffer, "AGGETTIVO")==0) {
            use_adjectives=true;
            dictionary[cw].t=ADJECTIVE;
            if(dictionary[cw].code>255)
                adjective_as_byte=false;
        } else {
            fprintf(stderr,"Unknown word type: %s.\n",buffer);
            free(dictionary);
            return NULL;
        }
    }
    return dictionary;
}

/** Get the dictionary size.
*/
unsigned int get_dict_size(FILE *f)
{
    fpos_t pos;
    unsigned int counter=0;

    while(fscanf(f,"%80s",buffer)==1){
        if(strcmp(buffer,"DIZIONARIO")==0) {
            break;
        }
    }
    fgetpos(f, &pos);
    while(fscanf(f,"%80s",buffer)==1){
        if(strcmp(buffer,"LOCAZIONI")==0) {
            break;
        }
        ++counter;
    }
    fsetpos(f, &pos);
    return counter/3;
}

/** Load a line from a file, store it in the buffer and remove the newline.
*/
char *getlinep(FILE *f)
{
    unsigned int sl;
    if(fgets(buffer, BUFFERSIZE, f) == NULL) {
        return NULL;
    }
    sl=strlen(buffer);
    if(sl>=2 && buffer[sl - 2]=='\r')    // This is needed for Windows
        buffer[sl - 2] = '\0';  // style newline code (\r\n).
    else if(sl>=1)
        buffer[sl - 1] = '\0';
    return buffer;
}

/** Get the number of the rooms in the game.
*/
int get_room_number(FILE *f)
{
    fpos_t pos;
    int counter=0;
    while(getlinep(f)){
        if(strcmp(buffer,"LOCAZIONI")==0) {
            break;
        }
    }
    fgetpos(f, &pos);
    while (getlinep(f)) {
        if(strcmp(buffer,"MESSAGGI")==0) {
            break;
        }
        ++counter;
    }
    fsetpos(f, &pos);
    return counter/14;
}

/** Read all the "conditions" in the file.
*/
char **read_cond(FILE*f, int size)
{
    int i;
    char **cond;
    char *errorp="Could not allocate enough memory for the conditions.\n";

    if(size<=0)
        return NULL;

    if (verbose)
        printf("\n");
    cond = (char**)calloc(size, sizeof(char*));
    if (cond==NULL) {
        printf("%s",errorp);
        return NULL;
    }
    for(i=0; i<size;++i) {
        if(getlinep(f)==NULL) {
            return NULL;
        }
        if (verbose)
            printf("[%s]\n",buffer);
        cond[i]=calloc(strlen(buffer)+1,sizeof(char));
        if (cond[i]==NULL) {
            printf("%s",errorp);
            return NULL;
        }
        strcpy(cond[i],buffer);
    }
    return cond;
}

/** Read all the "local conditions" in the file.
*/
localc* read_local(FILE*f, int size)
{
    unsigned int i,r;
    localc* cond;
    char *errorp="Could not allocate enough memory for the local conditions.\n";

    if(size<=0)
        return NULL;

    cond = (localc*)calloc(size, sizeof(localc));
    if (cond==NULL) {
        printf("%s",errorp);
        return NULL;
    }
    if(verbose)
        printf("\n");
    for(i=0; i<size;++i) {
        getlinep(f);
        if(sscanf(buffer,"%d",&r)!=1) {
            printf("\nCould not read the room number! Read [%s]\n",buffer);
            return NULL;
        }
        cond[i].room=r;
        getlinep(f);
        cond[i].condition=calloc(strlen(buffer)+1,sizeof(char));
        if (cond[i].condition==NULL) {
            printf("%s",errorp);
            return NULL;
        }
        strcpy(cond[i].condition,buffer);
        if(verbose)
            printf("Local room %d [%s]\n", cond[i].room, cond[i].condition);
    }
    return cond;
}

/** Get the rooms in the game
*/
room* read_rooms(FILE *f, int size)
{
    char *errorp="Could not allocate enough memory for the room description.\n";
    int i,j;
    room *world = (room*)calloc(size, sizeof(room));
    if (world==NULL) {
        printf("%s",errorp);
        return NULL;
    }

    for(i=0; i<size;++i) {
        getlinep(f);  // Read line 1
        sscanf(buffer, "%d",&(world[i].code));
        getlinep(f);  // Read line 2
        world[i].long_d=calloc(strlen(buffer)+1,sizeof(char));
        strcpy(world[i].long_d,buffer);
        if(verbose)
            printf("Room %d [%s]\n",world[i].code,world[i].long_d);
        if(compress_descriptions==true)
            analyze(encodechar(world[i].long_d));
        getlinep(f);  // Read line 3
        world[i].s=calloc(strlen(buffer)+1,sizeof(char));
        strcpy(world[i].s,buffer);
        if(compress_descriptions==true)
            analyze(encodechar(world[i].s));
        getlinep(f);  // Read line 4
        world[i].short_d=calloc(strlen(buffer)+1,sizeof(char));
        strcpy(world[i].short_d,buffer);
        if(compress_descriptions==true)
            analyze(encodechar(world[i].short_d));
        for(j=0;j<10;++j) {
            getlinep(f);
            if(sscanf(buffer,"%d",&(world[i].directions[j]))!=1) {
                printf("Error reading directions.\n");
                printf("Object code %d, direction %d\n",world[i].code,j+1);
                getlinep(f);
                printf("line [%s]\n",buffer);
                return NULL;
            }
        }
    }
    return world;
}

/** Get the rooms in the game
*/
object* read_objects(FILE *f, int size)
{
    char *errorp="Could not allocate enough memory for the objects.\n";
    unsigned int i,j;
    if(size<=0)
        return NULL;
    object *obj = (object*)calloc(size, sizeof(object));
    if (obj==NULL) {
        printf("%s",errorp);
        return NULL;
    }

    if(verbose)
        printf("\n");

    for(i=0; i<size;++i) {
        getlinep(f);
        sscanf(buffer, "%d",&(obj[i].code));

        getlinep(f);
        obj[i].s=calloc(strlen(buffer)+1,sizeof(char));
        strcpy(obj[i].s,buffer);

        getlinep(f);
        obj[i].desc=calloc(strlen(buffer)+1,sizeof(char));
        strcpy(obj[i].desc,buffer);
        if(compress_descriptions==true)
            analyze(encodechar(obj[i].desc));

        getlinep(f);
        sscanf(buffer, "%d",&(obj[i].weight));

        getlinep(f);
        sscanf(buffer, "%d",&(obj[i].size));

        getlinep(f);
        sscanf(buffer, "%d",&(obj[i].position));

        getlinep(f);
        if(strcmp(buffer,"FALSE")) {
    //        obj[i].attributes|=ISNOTMOVABLE;
        } else {
            obj[i].attributes|=ISNOTMOVABLE;
        }

        getlinep(f);
        if(strcmp(buffer,"TRUE")) {
 //           obj[i].isnotwereable=false;
        } else {
            obj[i].attributes|=ISWEREABLE;
        }
        if(verbose)
            printf("Object %d [%s]\n",obj[i].code,obj[i].desc);
    }
    return obj;
}

info *read_header(FILE *f)
{
    char *errorp="Could not allocate enough memory for header info.\n";
    info *in = (info*)malloc(sizeof(info));
    getlinep(f);
    if(strcmp(buffer,"AWS")!=0) {
        printf("This is not an AWS file!\n");
        return NULL;
    }
    getlinep(f);
    getlinep(f);
    in->version=calloc(strlen(buffer)+1,sizeof(char));
    strcpy(in->version,buffer);

    getlinep(f);
    sscanf(buffer, "%d",&(in->textcolor));

    getlinep(f);
    sscanf(buffer, "%d",&(in->backcolor));

    getlinep(f);
    sscanf(buffer, "%d",&(in->textcolordark));

    getlinep(f);
    sscanf(buffer, "%d",&(in->backcolordark));

    getlinep(f);
    in->name=calloc(strlen(buffer)+1,sizeof(char));
    strcpy(in->name,buffer);

    getlinep(f);
    in->author=calloc(strlen(buffer)+1,sizeof(char));
    strcpy(in->author,buffer);

    getlinep(f);
    in->date=calloc(strlen(buffer)+1,sizeof(char));
    strcpy(in->date,buffer);

    getlinep(f);
    in->description=calloc(strlen(buffer)+1,sizeof(char));
    strcpy(in->description,buffer);

    getlinep(f);
    sscanf(buffer, "%d",&(in->code));

    getlinep(f);
    in->fontname=calloc(strlen(buffer)+1,sizeof(char));
    strcpy(in->fontname,buffer);

    getlinep(f);
    sscanf(buffer, "%d",&(in->charsize));

    getlinep(f);
    sscanf(buffer, "%d",&(in->fontstyle));

    getlinep(f);
    sscanf(buffer, "%d",&(in->startroom));

    getlinep(f);
    if(strcmp(buffer,"FALSE")) {
        in->graphical=false;
    } else {
        in->graphical=true;
    }

    getlinep(f);
    sscanf(buffer, "%d",&(in->maxcarryingw));

    getlinep(f);
    sscanf(buffer, "%d",&(in->maxcarryings));

    if(compress_messages==true) {
        analyze(encodechar(in->name));
        analyze(encodechar(in->author));
        analyze(encodechar(in->date));
        analyze(encodechar(in->description));
    }

    return in;
}

/** Get the messages in the game
*/
message* read_messages(FILE *f, int size)
{
    char *errorp="Could not allocate enough memory for messages.\n";
    unsigned int i,j;
    message *msg = (message*)calloc(size, sizeof(message));
    if (msg==NULL) {
        printf("%s",errorp);
        return NULL;
    }
    if(verbose)
        printf("\n");

    for(i=0; i<size;++i) {
        getlinep(f);
        sscanf(buffer, "%d",&(msg[i].code));

        getlinep(f);
        msg[i].txt=calloc(strlen(buffer)+1,sizeof(char));
        strcpy(msg[i].txt,buffer);
        if(compress_messages==true)
            analyze(encodechar(msg[i].txt));
        if(verbose)
            printf("Message %d [%s]\n",msg[i].code,msg[i].txt);
    }
    if(compress_messages==true)
        analyze(AREYOUSURE);
        analyze(EXITRESTART);
    return msg;
}

/** Get the number of the objects in the game.
*/
unsigned int get_objects_number(FILE *f)
{
    fpos_t pos;
    unsigned int counter=0;
    unsigned int sl=0;
    while(getlinep(f)){
        if(strcmp(buffer,"OGGETTI")==0) {
            break;
        }
    }
    fgetpos(f, &pos);
    while (getlinep(f)) {
        if(strcmp(buffer,"FINEDATI")==0) {
            break;
        }
        ++counter;
    }
    fsetpos(f, &pos);
    return counter/8;
}

/** Get the number of messages in the game.
*/
int get_messages_number(FILE *f)
{
    fpos_t pos;
    int counter=0;
    while(getlinep(f)){
        if(strcmp(buffer,"MESSAGGI")==0) {
            break;
        }
    }
    fgetpos(f, &pos);
    while (getlinep(f)) {
        if(strcmp(buffer,"OGGETTI")==0) {
            break;
        }
        ++counter;
    }
    fsetpos(f, &pos);
    return counter/2;
}

/** Get the number of "high conditions" in the file.
*/
int get_hi_cond_size(FILE *f)
{
    fpos_t pos;
    int counter=0;
    while (getlinep(f)) {
        if(strcmp(buffer,"CONDIZIONIHI")==0) {
            break;
        }
    }
    fgetpos(f, &pos);
    while (getlinep(f)) {
        if(strcmp(buffer,"CONDIZIONILOW")==0) {
            break;
        }
        ++counter;
    }
    fsetpos(f, &pos);
    return counter;
}

/** Get the number of "low conditions" in the file.
*/
int get_low_cond_size(FILE *f)
{
    fpos_t pos;
    int counter=0;
    while(getlinep(f)){
        if(strcmp(buffer,"CONDIZIONILOW")==0) {
            break;
        }
    }
    fgetpos(f, &pos);
    while (getlinep(f)) {
        if(strcmp(buffer,"CONDIZIONILOCALI")==0) {
            break;
        }
        ++counter;
    }
    fsetpos(f, &pos);
    return counter;
}
/** Get the number of "local conditions" in the file.
*/
int get_local_cond_size(FILE *f)
{
    fpos_t pos;
    int counter=0;
    while(getlinep(f)){
        if(strcmp(buffer,"CONDIZIONILOCALI")==0) {
            break;
        }
    }
    fgetpos(f, &pos);
    while (getlinep(f)) {
        if(strcmp(buffer,"DIZIONARIO")==0) {
            break;
        }
        ++counter;
    }
    fsetpos(f, &pos);
    return (counter)/2;
}


char token[BUFFERSIZE];
char next[BUFFERSIZE];


/** Get a new token (store it in the shared variable "token") and
    return the new position in the line.
*/
unsigned int get_token(char *line, unsigned int pos)
{
    char c;
    unsigned int k=0;
    while(line[pos]==' ')
        ++pos;

    for(;(c=line[pos])!='\0' && c!=' ';++pos) {
        if(c>='a' && c<='z')
            c-=32;  // Convert to uppercase
        token[k++]=c;
    }
    token[k]='\0';
    return pos+1;
}

/** Give a peek to the next token (store it in the shared variable "next").
*/
unsigned int peek_token(char *line, unsigned int pos)
{
    char c;
    unsigned int k=0;
    while(line[pos]==' ')
        ++pos;

    for(;(c=line[pos])!='\0' && c!=' ';++pos) {
        if(c>='a' && c<='z')
            c-=32;  // Convert to uppercase
        next[k++]=c;
    }
    next[k]='\0';
    return pos+1;
}

void strcon(char *str1, const char* str2)
{
    unsigned int i,j;
    char c;

    for(i=0;str1[i]!='\0'&&i<BUFFERSIZE;++i) ;

    for(j=0;(c=str2[j])!='\0'&&i<BUFFERSIZE;++j)
        str1[i++]=str2[j];

    str1[i]='\0';
}

unsigned int process_functions(char *line, unsigned int scanpos)
{
    unsigned int value;
    scanpos=get_token(line, scanpos);
    if(strcmp(token,"NO1")==0) {
        strcon(function_res,"noun1");
    } else if(strcmp(token,"CTR")==0) {
        strcon(function_res,"counter[");
        scanpos=process_functions(line,scanpos);
        strcon(function_res,"]");
    } else if(strcmp(token,"VBNO")==0) {
        strcon(function_res,"verb");
    } else if(strcmp(token,"NO2")==0) {
        strcon(function_res,"noun2");
    } else if(strcmp(token,"ADJENO")==0||strcmp(token,"ADVENO")==0) {
        strcon(function_res,"adve");
    } else if(strcmp(token,"ROOM")==0) {
        strcon(function_res,"current_position");
    } else if(strcmp(token,"OBJLOC")==0) {
        strcon(function_res,"get_object_position(");
        scanpos=process_functions(line,scanpos);
        strcon(function_res,")");
    } else if(strcmp(token,"WEIG")==0) {
        strcon(function_res,"search_object_p(");
        scanpos=process_functions(line,scanpos);
        strcon(function_res,")->weight");
        if(dont_care_size_weight) {
            fprintf(stderr,
                "WARNING: selected -w option and adventure exploits WEIGH!\n");
        }
    } else if(strcmp(token,"ACTORNO")==0) {
        strcon(function_res,"actor");
    } else {
        if(sscanf(token, "%d",&value)==1) {
            sprintf(token, "%d", value);
            strcon(function_res,token);
        } else {
            printf("Function not recognized %s in line\n[%s]\n", token,line);
            ++no_of_errors;
        }

    }
    return scanpos;
}


/*  Decisions */

/** AT */
unsigned int decision_at(FILE *f, char *line, unsigned int scanpos)
{
    boolean proc=false;
    unsigned int sp;
    int val1;
    boolean polarity=true;
    char *arg1=NULL,*arg2=NULL;

    start_function();
    scanpos=process_functions(line, scanpos);
    arg1=(char *) calloc(strlen(function_res)+1,sizeof(char));
    if(arg1==NULL) {
        printf("Error allocating memory!\n");
        exit(1);
    }
    strcpy(arg1,function_res);
    sp=peek_token(line, scanpos);
    if(shortcuts==true&&(strcmp(next,"AND")==0)) {
        sp=peek_token(line, sp);
        if(strcmp(next,"SET?")==0||strcmp(next,"RES?")==0) {
            if(strcmp(next,"RES?")==0)
                polarity=false;
            start_function();
            sp=process_functions(line, sp);
            arg2=(char *) calloc(strlen(function_res)+1,sizeof(char));
            if(arg2==NULL) {
                printf("Error allocating memory!\n");
                exit(1);
            }
            strcpy(arg2,function_res);
            sp=peek_token(line, sp);
            if(strcmp(next,"THEN")==0) {
                sp=peek_token(line, sp);
                if(strcmp(next,"MESS")==0) {
                    start_function();
                    sp=process_functions(line, sp);
                    peek_token(line, sp);
                    if(strcmp(next,"ENDIF")==0) {
                        scanpos=sp;
                        proc=true;
                        complete_shortcut=true;
                        need_amsm=true;
                        if(hardcoded_messages==false)
                            fprintf(f, "1) amsm(%s,%s,%d,%s);\n\n",
                                arg1,arg2,polarity,function_res);
                        else
                            fprintf(f, "1) amsm(%s,%s,%d,message%s);\n\n",
                                arg1,arg2,polarity,function_res);
                    }
                }/*  else {
                    if (polarity) {
                        need_as=true;
                        proc=true;
                        fprintf(f, "check_position_marker_on(%s,%s)", arg1,arg2);
                    } else {
                        need_ar=true;
                        proc=true;
                        fprintf(f, "check_position_marker_off(%s,%s)", arg1,arg2);
                    }
                }*/
            } /*else {
                if (polarity) {
                    need_as=true;
                    proc=true;
                    fprintf(f, "check_position_marker_on(%s,%s)", arg1,arg2);
                } else {
                    need_ar=true;
                    proc=true;
                    fprintf(f, "check_position_marker_off(%s,%s)", arg1,arg2);
                }
            }*/
        }
    }
    if(proc==false) {
        // This actually increases the size of the executable (C128)
        /* if(sscanf(arg1, "%d",&val1)==1 && val1<256) {
            fprintf(f, "cpi(%s)", arg1);
            need_cpi=true;
        } else {*/
        fprintf(f, "current_position==%s", arg1);
        //}
    }
    if(arg1!=NULL)
        free(arg1);
    if(arg2!=NULL)
        free(arg2);
    return scanpos;
}
/** NOTAT */
unsigned int decision_notat(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "current_position!=%s", function_res);
    return scanpos;
}

/** SET? */
unsigned int decision_set(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "marker[%s]", function_res);
    return scanpos;
}

/** RES? */
unsigned int decision_res(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "marker[%s]==false", function_res);
    return scanpos;
}
/** ROOMGT */
unsigned int decision_roomgt(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "current_position>%s", function_res);
    return scanpos;
}
/** ROOMLT */
unsigned int decision_roomlt(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "current_position<%s", function_res);
    return scanpos;
}
/** CARR */
unsigned int decision_carr(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "object_is_carried(%s)", function_res);
    return scanpos;
}
/** NOTIN */
unsigned int decision_notin(FILE *f, char *line, unsigned int scanpos)
{
    char *arg1;
    unsigned int l;
    start_function();
    scanpos=process_functions(line, scanpos);
    l=strlen(function_res);
    arg1=(char *) calloc(l+1,sizeof(char));
    if(arg1==NULL) {
        printf("Error allocating memory!\n");
        exit(1);
    }
    strcpy(arg1,function_res);

    start_function();
    scanpos=process_functions(line, scanpos);

    fprintf(f, "get_object_position(%s)!=%s", arg1,function_res);
    free(arg1);
    return scanpos;
}
/** EQU? */
unsigned int decision_equ(FILE *f, char *line, unsigned int scanpos)
{
    char *arg1;
    unsigned int l;
    start_function();
    scanpos=process_functions(line, scanpos);
    l=strlen(function_res);
    arg1=(char *) calloc(l+1,sizeof(char));
    if(arg1==NULL) {
        printf("Error allocating memory!\n");
        exit(1);
    }
    strcpy(arg1,function_res);

    start_function();
    scanpos=process_functions(line, scanpos);

    fprintf(f, "counter[%s]==%s", arg1,function_res);
    free(arg1);
    return scanpos;
}
/** NOTEQU? */
unsigned int decision_notequ(FILE *f, char *line, unsigned int scanpos)
{
    char *arg1;
    unsigned int l;
    start_function();
    scanpos=process_functions(line, scanpos);
    l=strlen(function_res);
    arg1=(char *) calloc(l+1,sizeof(char));
    if(arg1==NULL) {
        printf("Error allocating memory!\n");
        exit(1);
    }
    strcpy(arg1,function_res);

    start_function();
    scanpos=process_functions(line, scanpos);

    fprintf(f, "counter[%s]!=%s", arg1,function_res);
    free(arg1);
    return scanpos;
}
/** VERB */
unsigned int decision_verb(FILE *f, char *line, unsigned int scanpos)
{
    unsigned int sp,at,tt, val1, val2, fval;
    boolean proc=false;
    char *arg1,*arg2;
    start_function();
    tt=scanpos=process_functions(line, scanpos);
    sp=peek_token(line, scanpos);
    /* A certain complexity comes from the search of shortcuts.
       One must peek into the next tokens to see if they correspond to
       something recognized.

       Here is how it is done: peek_token allows to load in "next" the new
       token recognized. Usually the parsing is done at the position sp, the
       value of scanpos is the current "valid" parsing position. Once a shortcut
       can be applied, scanpos is updated to the value of sp.

       In some extent, sp is "exploratory".

    */
    if(shortcuts==true&&(strcmp(next,"AND")==0)) {
        sp=peek_token(line, sp);
        if(strcmp(next,"ACTOR")==0) {
            arg1=(char *) calloc(strlen(function_res)+1,sizeof(char));
            if(arg1==NULL) {
                printf("Error allocating memory!\n");
                exit(1);
            }
            strcpy(arg1,function_res);
            scanpos=sp;
            start_function();
            scanpos=process_functions(line, scanpos);
            arg2=(char *) calloc(strlen(function_res)+1,sizeof(char));
            if(arg2==NULL) {
                printf("Error allocating memory!\n");
                exit(1);
            }
            strcpy(arg2,function_res);

            /*  75 is a very frequent action with an actor (SPEAK).
                This is done only if the actor has a 1-byte code. */
            if(strcmp(arg1,"75")==0 &&
                sscanf(arg2, "%d",&val2)==1 && val2<256)
            {
                sp=peek_token(line, scanpos);
                if(strcmp(next,"AND")==0) {
                    sp=peek_token(line, sp);
                    if(strcmp(next,"AVAI")==0) {
                        scanpos=sp;
                        start_function();
                        scanpos=process_functions(line, scanpos);
                        if(strcmp(function_res,arg2)==0) {
                            fprintf(f, "check_verb75_actor_available(%s)",
                                arg2);
                        } else {
                            fprintf(f, "check_verb75_actor(%s)&&"
                                "object_is_available(%s)",
                                arg2, function_res);
                            need_check_verb_actor=true;
                        }
                    } else {
                        fprintf(f, "check_verb75_actor(%s)", arg2);
                    }
                } else {
                    fprintf(f, "check_verb75_actor(%s)", arg2);
                }
            } else if(strcmp(arg1,"70")==0 && /* 70 is EXAMINE */
                sscanf(arg2, "%d",&val2)==1 && val2<256)
            {
                sp=peek_token(line, scanpos);
                if(strcmp(next,"AND")==0) {
                    sp=peek_token(line, sp);
                    if(strcmp(next,"AVAI")==0) {
                        scanpos=sp;
                        start_function();
                        scanpos=process_functions(line, scanpos);
                        if(strcmp(function_res,arg2)==0) {
                            fprintf(f, "check_verb70_actor_available(%s)",
                                arg2);
                        } else {
                            fprintf(f, "check_verb70_actor(%s)&&"
                                "object_is_available(%s)",
                                arg2, function_res);
                            need_check_verb_actor=true;
                        }
                    } else {
                        fprintf(f, "check_verb70_actor(%s)", arg2);
                    }
                } else {
                    fprintf(f, "check_verb70_actor(%s)", arg2);
                }
            } else {
                fprintf(f, "check_verb_actor(%s,%s)", arg1,arg2);
            }
            need_check_verb_actor=true;
            free(arg1);
            free(arg2);
        } else if(strcmp(next,"NOUN")==0) {
            arg1=(char *) calloc(strlen(function_res)+1,sizeof(char));
            if(arg1==NULL) {
                printf("Error allocating memory!\n");
                exit(1);
            }
            strcpy(arg1,function_res);
            scanpos=sp;
            start_function();
            scanpos=process_functions(line, scanpos);
            arg2=(char *) calloc(strlen(function_res)+1,sizeof(char));
            if(arg2==NULL) {
                printf("Error allocating memory!\n");
                exit(1);
            }
            strcpy(arg2,function_res);

            sp=peek_token(line, scanpos);
            if(strcmp(next,"AND")==0) {
                sp=peek_token(line, sp);
                if(strcmp(next,"AVAI")==0) {
                    scanpos=sp;
                    start_function();
                    scanpos=process_functions(line, scanpos);
                    if(strcmp(arg1,"70")==0 &&
                        sscanf(arg2, "%d",&val2)==1 && val2<256 &&
                        sscanf(function_res, "%d",&fval)==1 && fval<256)
                    {
                        if(strcmp(arg2,function_res)==0)
                            fprintf(f,"cvna70neq(%s)", arg2);
                        else
                            fprintf(f,"cvna70(%s,%s)", arg2, function_res);
                    } else {
                        fprintf(f,"cvna(%s,%s,%s)", arg1, arg2, function_res);
                    }
                    need_cvna=true;
                    proc=true;
                }
            } else if(strcmp(next,"OR")==0) {
                if(sscanf(arg1, "%d",&val1)==1 && val1<256)
                    fprintf(f, "cv(%s)", arg1);
                else
                    fprintf(f, "verb==%s", arg1);

                need_cv=true;
                scanpos=tt;
                proc=true;
            } else if(strcmp(next,"THEN")==0) {
                int lp=sp-5;        // beurk!

                sp=peek_token(line, sp);
            }
            if(proc==false) {

                if(strcmp(arg1,"70")==0 &&
                    sscanf(arg2, "%d",&val2)==1 && val2<256)
                {
                    /* 70 (EXAMINE) is by far the most frequent verb.
                   this is done only if the noun has a code <256. */
                    fprintf(f, "cvn70(%s)", arg2);
                } else {
                    fprintf(f, "check_verb_noun(%s,%s)", arg1,arg2);
                }
                need_cvn=true;
            }
            if(arg1!=NULL) free(arg1);
            if(arg2!=NULL) free(arg2);

        } else {
            if(sscanf(function_res, "%d",&val1)==1 && val1<256)
                fprintf(f, "cv(%s)", function_res);
            else
                fprintf(f, "verb==%s", function_res);

            need_cv=true;
        }
    } else if(shortcuts==true&&(strcmp(next,"OR")==0)) {
        sp=peek_token(line, sp);
        if(strcmp(next,"VERB")==0||strcmp(next,"VBNOEQ")==0) {
            arg1=(char *) calloc(strlen(function_res)+1,sizeof(char));
            if(arg1==NULL) {
                printf("Error allocating memory!\n");
                exit(1);
            }
            strcpy(arg1,function_res);
            scanpos=sp;
            start_function();
            scanpos=process_functions(line, scanpos);
            arg2=(char *) calloc(strlen(function_res)+1,sizeof(char));
            if(arg2==NULL) {
                printf("Error allocating memory!\n");
                exit(1);
            }
            strcpy(arg2,function_res);
            sp=peek_token(line, scanpos);
            if(strcmp(next,"AND")==0) {
                sp=peek_token(line, sp);
                if(strcmp(next,"NOUN")==0) {
                    int sp1=peek_token(line, sp);
                    if(strcmp(next,"OR")==0) {
                        proc=false;
                    } else {
                        proc=true;
                        scanpos=sp;
                        start_function();
                        scanpos=process_functions(line, scanpos);
                        if(strcmp(arg1,"100")==0 && strcmp(arg2,"0")==0 &&
                            sscanf(function_res, "%d",&val1)==1 && val1<256)
                        {
                            fprintf(f,"vovn100_0(%s)", function_res);
                        } else {
                            fprintf(f,"vovn(%s,%s,%s)", arg1,
                                arg2,function_res);
                        }
                        need_vovn=true;
                    }
                }
            }
            if(proc==false) {
                int sp1=peek_token(line, sp);
                if(strcmp(next,"OR")==0) {
                    proc=false;
                } else {
                    fprintf(f,"vov(%s,%s)", arg1, arg2);
                    need_vov=true;
                }
            }
            free(arg1);
            free(arg2);
        }
    } else {
        if(shortcuts==true &&sscanf(function_res, "%d",&val1)==1 && val1<256) {
            fprintf(f, "cv(%s)", function_res);
            need_cv=true;
        } else {
            fprintf(f, "verb==%s", function_res);
        }
    }
    return scanpos;
}
/** VBNOGT */
unsigned int decision_vbnogt(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "verb>%s", function_res);
    return scanpos;
}
/** VBNOLT */
unsigned int decision_vbnolt(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "verb<%s", function_res);
    return scanpos;
}
/** NOUN */
unsigned int decision_noun(FILE *f, char *line, unsigned int scanpos)
{
    char *arg1;
    unsigned int sp,at;

    start_function();
    scanpos=process_functions(line, scanpos);
    sp=peek_token(line, scanpos);
    if(shortcuts==true&&(strcmp(next,"OR")==0)) {
        sp=peek_token(line, sp);
        if(strcmp(next,"NOUN")==0) {
            arg1=(char *) calloc(strlen(function_res)+1,sizeof(char));
            if(arg1==NULL) {
                printf("Error allocating memory!\n");
                exit(1);
            }
            strcpy(arg1,function_res);
            scanpos=sp;
            start_function();
            scanpos=process_functions(line, scanpos);
            fprintf(f,"non1(%s,%s)", arg1, function_res);
            need_non1=true;
            free(arg1);
        }
    } else {
        fprintf(f, "noun1==%s", function_res);
    }
    return scanpos;
}
/** ADVE */
unsigned int decision_adve(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "adve==%s", function_res);
    return scanpos;
}
/** NO1EQ */
unsigned int decision_no1eq(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "noun1==%s", function_res);
    return scanpos;
}
/** AVAI */
unsigned int decision_avai(FILE *f, char *line, unsigned int scanpos)
{
    unsigned int counter,value;
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "object_is_available(%s)",function_res);
    return scanpos;
}
/** NOTAVAI */
unsigned int decision_notavai(FILE *f, char *line, unsigned int scanpos)
{
    unsigned int counter,value;
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "!object_is_available(%s)",function_res);

    /*fprintf(f, "(get_object_position(%s)!=current_position)&&",
        function_res);
    fprintf(f, "(object_is_carried(%s)==false)", function_res);*/

    return scanpos;
}
/** NO2EQ */
unsigned int decision_no2eq(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "noun2==%s", function_res);
    return scanpos;
}
/** NOTCARR */
unsigned int decision_notcarr(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "object_is_carried(%s)==false", function_res);
    return scanpos;
}
/** NO1GT */
unsigned int decision_no1gt(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "noun1>%s", function_res);
    checked_noun1_greater_zero=true;
    return scanpos;
}
/** NO1LT */
unsigned int decision_no1lt(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    if(checked_noun1_greater_zero) {
        fprintf(f, "noun1<%s", function_res);
    } else {
        fprintf(f, "(noun1>0&&noun1<%s)", function_res);
    }
    return scanpos;
}
/** NO2GT */
unsigned int decision_no2gt(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "noun2>%s", function_res);
    return scanpos;
}
/** NO2LT */
unsigned int decision_no2lt(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "noun2>0&&noun2<%s", function_res);
    return scanpos;
}
/** CTRGT */
unsigned int decision_ctrgt(FILE *f, char *line, unsigned int scanpos)
{
    char *arg1;
    unsigned int l;
    start_function();
    scanpos=process_functions(line, scanpos);
    l=strlen(function_res);
    arg1=(char *) calloc(l+1,sizeof(char));
    if(arg1==NULL) {
        printf("Error allocating memory!\n");
        exit(1);
    }
    strcpy(arg1,function_res);

    start_function();
    scanpos=process_functions(line, scanpos);

    fprintf(f, "counter[%s]>%s", arg1,function_res);
    free(arg1);
    return scanpos;
}
/** CTRLT */
unsigned int decision_ctrlt(FILE *f, char *line, unsigned int scanpos)
{
    char *arg1;
    unsigned int l;
    start_function();
    scanpos=process_functions(line, scanpos);
    l=strlen(function_res);
    arg1=(char *) calloc(l+1,sizeof(char));
    if(arg1==NULL) {
        printf("Error allocating memory!\n");
        exit(1);
    }
    strcpy(arg1,function_res);

    start_function();
    scanpos=process_functions(line, scanpos);

    fprintf(f, "counter[%s]<%s", arg1,function_res);
    free(arg1);
    return scanpos;
}
/** IN */
unsigned int decision_in(FILE *f, char *line, unsigned int scanpos)
{
    char *arg1;
    unsigned int l;
    start_function();
    scanpos=process_functions(line, scanpos);
    l=strlen(function_res);
    arg1=(char *) calloc(l+1,sizeof(char));
    if(arg1==NULL) {
        printf("Error allocating memory!\n");
        exit(1);
    }
    strcpy(arg1,function_res);

    start_function();
    scanpos=process_functions(line, scanpos);

    fprintf(f, "get_object_position(%s)==%s", arg1, function_res);
    free(arg1);
    return scanpos;
}
/** HERE */
unsigned int decision_here(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "object_is_here(%s)",
        function_res);
    return scanpos;
}
/** NOTHERE */
unsigned int decision_nothere(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "object_is_here(%s)==false",
        function_res);
    return scanpos;
}
/** PROB */
unsigned int decision_prob(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "(rand()%%100)<%s", function_res);
    return scanpos;
}
/** ISMOVABLE */
unsigned int decision_ismovable(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "(search_object_p(%s)->attributes&ISNOTMOVABLE)==false",
        function_res);
    return scanpos;
}
/** ISNOTMOVABLE */
unsigned int decision_isnotmovable(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "(search_object_p(%s)->attributes&ISNOTMOVABLE)",
        function_res);
    return scanpos;
}
/** ISWEARABLE */
unsigned int decision_iswearable(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "(search_object_p(%s)->attributes&ISWEREABLE)!=0",
        function_res);
    return scanpos;
}
/** ISNOTWEARABLE */
unsigned int decision_isnotwearable(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "search_object_p(%s)->attributes&ISWEREABLE==0",
        function_res);
    return scanpos;
}
/** ISWEARING */
unsigned int decision_iswearing(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "get_object_position(%s)==WEARED",
        function_res);
    return scanpos;
}
/** ISNOTWEARING */
unsigned int decision_isnotwearing(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "get_object_position(%s)!=WEARED",
        function_res);
    return scanpos;
}
/** ISCARRSOME */
unsigned int decision_iscarrsome(FILE *f, char *line, unsigned int scanpos)
{
    fprintf(f, "iscarrsome()");
    need_iscarrsome=true;
    return scanpos;
}
/** ISCARRNOTH */
unsigned int decision_iscarrnoth(FILE *f, char *line, unsigned int scanpos)
{
    fprintf(f, "!iscarrsome()");
    need_iscarrsome=true;
    return scanpos;
}
/** ISWEARSOME */
unsigned int decision_iswearsome(FILE *f, char *line, unsigned int scanpos)
{
    fprintf(f, "iswearsome()");
    need_iswearsome=true;
    return scanpos;
}
/** ISWEARNOTH */
unsigned int decision_iswearnoth(FILE *f, char *line, unsigned int scanpos)
{
    fprintf(f, "!iswearsome()");
    need_iswearsome=true;
    return scanpos;
}
/** OBJLOCEQ */
unsigned int decision_objloceq(FILE *f, char *line, unsigned int scanpos)
{
    char *arg1;
    unsigned int l;
    start_function();
    scanpos=process_functions(line, scanpos);
    l=strlen(function_res);
    arg1=(char *) calloc(l+1,sizeof(char));
    if(arg1==NULL) {
        printf("Error allocating memory!\n");
        exit(1);
    }
    strcpy(arg1,function_res);

    start_function();
    scanpos=process_functions(line, scanpos);

    fprintf(f, TAB TAB "get_object_position(%s)==%s",
        arg1, function_res);
    free(arg1);
    return scanpos;
}
/** OBJLOCGT */
unsigned int decision_objlocgt(FILE *f, char *line, unsigned int scanpos)
{
    char *arg1;
    unsigned int l;
    start_function();
    scanpos=process_functions(line, scanpos);
    l=strlen(function_res);
    arg1=(char *) calloc(l+1,sizeof(char));
    if(arg1==NULL) {
        printf("Error allocating memory!\n");
        exit(1);
    }
    strcpy(arg1,function_res);

    start_function();
    scanpos=process_functions(line, scanpos);

    fprintf(f, TAB TAB "get_object_position(%s)>%s",
        arg1, function_res);
    free(arg1);
    return scanpos;
}
/** OBJLOCLT */
unsigned int decision_objloclt(FILE *f, char *line, unsigned int scanpos)
{
    char *arg1;
    unsigned int l;
    start_function();
    scanpos=process_functions(line, scanpos);
    l=strlen(function_res);
    arg1=(char *) calloc(l+1,sizeof(char));
    if(arg1==NULL) {
        printf("Error allocating memory!\n");
        exit(1);
    }
    strcpy(arg1,function_res);

    start_function();
    scanpos=process_functions(line, scanpos);

    fprintf(f, TAB TAB "get_object_position(%s)<%s",
        arg1, function_res);
    free(arg1);
    return scanpos;
}
/** ACTOR */
unsigned int decision_actor(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "actor==%s", function_res);
    return scanpos;
}
/** ACTORGT */
unsigned int decision_actorgt(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "actor>%s", function_res);
    return scanpos;
}
/** ACTORLT */
unsigned int decision_actorlt(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "actor<%s", function_res);
    return scanpos;
}
/** ADJE */
unsigned int decision_adje(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "adjective==%s", function_res);
    return scanpos;
}
/** ADJEGT */
unsigned int decision_adjegt(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "adjective>%s", function_res);
    return scanpos;
}
/** ADJELT */
unsigned int decision_adjelt(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "adjective<%s", function_res);
    return scanpos;
}
/** CONNEQ */
unsigned int decision_conneq(FILE *f, char *line, unsigned int scanpos)
{
    char *arg1, *arg2;
    unsigned int l;
    start_function();
    scanpos=process_functions(line, scanpos);
    l=strlen(function_res);
    arg1=(char *) calloc(l+1,sizeof(char));
    if(arg1==NULL) {
        printf("Error allocating memory!\n");
        exit(1);
    }
    strcpy(arg1,function_res);

    start_function();
    scanpos=process_functions(line, scanpos);
    l=strlen(function_res);
    arg2=(char *) calloc(l+1,sizeof(char));
    if(arg2==NULL) {
        printf("Error allocating memory!\n");
        exit(1);
    }
    strcpy(arg2,function_res);

    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "world[search_room(%s)].directions[(%s)-1]==%s",
        arg1, arg2, function_res);
    free(arg1);
    free(arg2);
    return scanpos;
}
/** CONNGT */
unsigned int decision_conngt(FILE *f, char *line, unsigned int scanpos)
{
    char *arg1, *arg2;
    unsigned int l;
    start_function();
    scanpos=process_functions(line, scanpos);
    l=strlen(function_res);
    arg1=(char *) calloc(l+1,sizeof(char));
    if(arg1==NULL) {
        printf("Error allocating memory!\n");
        exit(1);
    }
    strcpy(arg1,function_res);

    start_function();
    scanpos=process_functions(line, scanpos);
    l=strlen(function_res);
    arg2=(char *) calloc(l+1,sizeof(char));
    if(arg2==NULL) {
        printf("Error allocating memory!\n");
        exit(1);
    }
    strcpy(arg2,function_res);

    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "world[search_room(%s)].directions[(%s)-1]>%s",
        arg1, arg2, function_res);
    free(arg1);
    free(arg2);
    return scanpos;
}
/** CONNLT */
unsigned int decision_connlt(FILE *f, char *line, unsigned int scanpos)
{
    char *arg1, *arg2;
    unsigned int l;
    start_function();
    scanpos=process_functions(line, scanpos);
    l=strlen(function_res);
    arg1=(char *) calloc(l+1,sizeof(char));
    if(arg1==NULL) {
        printf("Error allocating memory!\n");
        exit(1);
    }
    strcpy(arg1,function_res);

    start_function();
    scanpos=process_functions(line, scanpos);
    l=strlen(function_res);
    arg2=(char *) calloc(l+1,sizeof(char));
    if(arg2==NULL) {
        printf("Error allocating memory!\n");
        exit(1);
    }
    strcpy(arg2,function_res);

    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "world[search_room(%s)].directions[(%s)-1]<%s",
        arg1, arg2, function_res);
    free(arg1);
    free(arg2);
    return scanpos;
}
/* Actions */

/** PRESSKEY */
unsigned int action_presskey(FILE *f, char *line, unsigned int scanpos)
{
    fprintf(f, TAB TAB "waitkey();\n");
    return scanpos;
}

/** GOTO */
unsigned int action_goto(FILE *f, char *line, unsigned int scanpos)
{
    unsigned int position;
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, TAB TAB "jump(%s);\n",function_res);
    //fprintf(f, TAB TAB "return 1;\n");
    fprintf(f, TAB TAB "return;\n");
    return scanpos;
}

/** SET */
unsigned int action_set(FILE *f, char *line, unsigned int scanpos)
{
    unsigned int position;
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, TAB TAB "marker[%s]=true;\n", function_res);
    return scanpos;
}

/** RESE */
unsigned int action_rese(FILE *f, char *line, unsigned int scanpos)
{
    unsigned int position;
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, TAB TAB "marker[%s]=false;\n", function_res);
    return scanpos;
}

/** CSET */
unsigned int action_cset(FILE *f, char *line, unsigned int scanpos)
{
    char *arg1;
    unsigned int l;
    start_function();
    scanpos=process_functions(line, scanpos);
    l=strlen(function_res);
    arg1=(char *) calloc(l+1,sizeof(char));
    if(arg1==NULL) {
        printf("Error allocating memory!\n");
        exit(1);
    }
    strcpy(arg1,function_res);

    start_function();
    scanpos=process_functions(line, scanpos);

    fprintf(f, TAB TAB "counter[%s]=%s;\n", arg1, function_res);
    free(arg1);
    return scanpos;
}

/** INCR */
unsigned int action_incr(FILE *f, char *line, unsigned int scanpos)
{
    unsigned int position, value;
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, TAB TAB "++counter[%s];\n", function_res);
    return scanpos;
}

/** DECR */
unsigned int action_decr(FILE *f, char *line, unsigned int scanpos)
{
    unsigned int position, value;
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, TAB TAB "if(counter[%s]>0) --counter[%s];\n", function_res,
            function_res);
    return scanpos;
}

/** MESS */
unsigned int action_mess(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    if(dont_issue_message)
        return scanpos;

    if(strcmp("1036",function_res)==0) {
        // Message 1036 is particular: it points to the name1 introduced by
        // the player
        fprintf(f, TAB TAB "printf(\"%%s\\n\",searchw(noun1));\n");
        if (compress5bit_dict || compress_hash_dict) {
            fprintf(stderr, "WARNING: the message 1036 is not compatible with "
                "the -5 option!");
        }
        need_searchw=true;
    } else  if(hardcoded_messages==false) {
       fprintf(f, TAB TAB "show_message(%s);\n",  function_res);
    } else {
        fprintf(f, TAB TAB "show_message(message%s);\n",  function_res);
    }

    return scanpos;
}
/** MESSNOLF */
unsigned int action_messnolf(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    if(hardcoded_messages==false)
        fprintf(f, TAB TAB "show_messagenlf(%s);\n",  function_res);
    else
        fprintf(f, TAB TAB "show_messagenlf(message%s);\n",  function_res);
    return scanpos;
}

/** BRIN */
unsigned int action_brin(FILE *f, char *line, unsigned int scanpos)
{
    unsigned int position, value;
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, TAB TAB "bring_object_here(%s);\n",
        function_res);
    return scanpos;
}

/** QUIT */
unsigned int action_quit(FILE *f, char *line, unsigned int scanpos)
{

    fprintf(f, TAB TAB "if(are_you_sure()) {\n");
    fprintf(f, TAB TAB TAB "leave(); exit(0);\n");
    fprintf(f, TAB TAB "}\n");
    fprintf(f, TAB TAB "return;\n");
    return scanpos;
}
/** EXIT */
unsigned int action_exit(FILE *f, char *line, unsigned int scanpos)
{
    need_checkexit=true;
    fprintf(f, TAB TAB "checkexit();\n");
    fprintf(f, TAB TAB "restart(); return;\n");
    return scanpos;
}
/** INVE */
unsigned int action_inve(FILE *f, char *line, unsigned int scanpos)
{
    fprintf(f, TAB TAB "inventory();\n");
    return scanpos;
}
/** MOVE */
unsigned int action_move(FILE *f, char *line, unsigned int scanpos, unsigned int dir)
{
    unsigned int position, value;
    /* This second version should work all the times. */
    fprintf(f, TAB TAB "move(%d);\n", dir-1);
    return scanpos;
}
/** LOOK */
unsigned int action_look(FILE *f, char *line, unsigned int scanpos)
{
    fprintf(f, TAB TAB "marker[120]=false;\n");
    return scanpos;
}
/** DROP */
unsigned int action_drop(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, TAB TAB "if(drop(%s)) return;\n",function_res);
    return scanpos;
}
/** TO */
unsigned int action_to(FILE *f, char *line, unsigned int scanpos)
{
    char *arg1;
    unsigned int l;
    start_function();
    scanpos=process_functions(line, scanpos);
    l=strlen(function_res);
    arg1=(char *) calloc(l+1,sizeof(char));
    if(arg1==NULL) {
        printf("Error allocating memory!\n");
        exit(1);
    }
    strcpy(arg1,function_res);

    start_function();
    scanpos=process_functions(line, scanpos);
    if(strcmp("0",function_res)==0) {
        fprintf(f, TAB TAB "set_object_position0(%s);\n", arg1);
    } else if(strcmp("1500",function_res)==0) {
        fprintf(f, TAB TAB "set_object_positionC(%s);\n", arg1);
    } else {
        fprintf(f, TAB TAB "set_object_position(%s,%s);\n",
            arg1, function_res);
    }
    free(arg1);
    return scanpos;
}
/** OKAY */
unsigned int action_okay(FILE *f, char *line, unsigned int scanpos)
{
    fprintf(f, TAB TAB "ok();\n");
    need_ok=true;
    //fprintf(f, TAB TAB "return 1;\n");
    fprintf(f, TAB TAB "return;\n");
    return scanpos;
}
/** PRIN */
unsigned int action_prin(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, TAB TAB "printf(\"%%d\\n\",%s);\n",function_res);
    return scanpos;
}
/** PRINNOLF */
unsigned int action_prinnolf(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, TAB TAB "printf(\"%%d\",%s);\n",function_res);
    return scanpos;
}
/** ADDC */
unsigned int action_addc(FILE *f, char *line, unsigned int scanpos)
{
    char *arg1;
    unsigned int l;
    start_function();
    scanpos=process_functions(line, scanpos);
    l=strlen(function_res);
    arg1=(char *) calloc(l+1,sizeof(char));
    if(arg1==NULL) {
        printf("Error allocating memory!\n");
        exit(1);
    }
    strcpy(arg1,function_res);

    start_function();
    scanpos=process_functions(line, scanpos);

    fprintf(f, TAB TAB "counter[%s]+=%s;\n", arg1, function_res);
    free(arg1);
    return scanpos;
}
/** HOLD */
unsigned int action_hold(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, TAB TAB "hold(%s);\n",function_res);
    need_hold=true;
    return scanpos;
}
/** GET */
unsigned int action_get(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, TAB TAB "if(get(%s)) return;\n", function_res);

    return scanpos;
}
/** GETALL */
unsigned int action_getall(FILE *f, char *line, unsigned int scanpos)
{
    fprintf(f, TAB TAB "for(dummy=0; dummy<OSIZE;++dummy)\n");
    fprintf(f, TAB TAB TAB "odummy=&obj[dummy];\n");
    fprintf(f, TAB TAB TAB "if(odummy->position==current_position");
    if(dont_care_size_weight==false) {
        fprintf(f,"&&counter[124]+odummy->size>counter[121]"
            "&&counter[120]+odummy->weight>counter[122]");
    }
    fprintf(f,") {\n");
    fprintf(f, TAB TAB TAB TAB "odummy->position=CARRIED;\n");
    if(dont_care_size_weight==false) {
        fprintf(f, TAB TAB TAB TAB "++counter[119];\n");
        fprintf(f, TAB TAB TAB TAB "counter[120]+=odummy->weight;\n");
        fprintf(f, TAB TAB TAB TAB "counter[124]+=odummy->size;\n");
    }
    fprintf(f, TAB TAB TAB "}\n");
    return scanpos;
}
/** DROPALL */
unsigned int action_dropall(FILE *f, char *line, unsigned int scanpos)
{
    fprintf(f, TAB TAB "for(dummy=0; dummy<OSIZE;++dummy){\n");
    fprintf(f, TAB TAB TAB "odummy=&obj[dummy];\n");
    fprintf(f, TAB TAB TAB
        "if(odummy->position==CARRIED) {\n");
    fprintf(f, TAB TAB TAB TAB
        "odummy->position=current_position;\n");
    if(dont_care_size_weight==false) {
        fprintf(f, TAB TAB TAB TAB "--counter[119];\n");
        fprintf(f, TAB TAB TAB TAB "counter[120]-=odummy->weight;\n");
        fprintf(f, TAB TAB TAB TAB  "counter[124]-=odummy->size;\n");
    }
    fprintf(f, TAB TAB TAB "}\n");
    fprintf(f, TAB TAB "}\n");
    return scanpos;
}
/** RESETALL */
unsigned int action_resetall(FILE *f, char *line, unsigned int scanpos)
{
    fprintf(f, TAB TAB "restart();\n");
    return scanpos;
}
/** SENDALLROOM */
unsigned int action_sendallroom(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, TAB TAB "sendallroom(%s);\n",function_res);
    need_sendallroom=true;
    return scanpos;
}

unsigned int search_room(unsigned int r)
{
    unsigned int idxl;
    for(idxl=0; idxl<rsize;++idxl)
        if(world[idxl].code==r)
            return idxl;
    return 0;
}

/** SETCONN */
unsigned int action_setconn(FILE *f, char *line, unsigned int scanpos)
{
    char *arg1, *arg2;
    unsigned int l;
    start_function();
    scanpos=process_functions(line, scanpos);
    l=strlen(function_res);
    arg1=(char *) calloc(l+1,sizeof(char));
    if(arg1==NULL) {
        printf("Error allocating memory!\n");
        exit(1);
    }
    strcpy(arg1,function_res);

    start_function();
    scanpos=process_functions(line, scanpos);
    l=strlen(function_res);
    arg2=(char *) calloc(l+1,sizeof(char));
    if(arg2==NULL) {
        printf("Error allocating memory!\n");
        exit(1);
    }
    strcpy(arg2,function_res);

    start_function();
    scanpos=process_functions(line, scanpos);
    if(sscanf(arg1,"%d",&l)==1 && l>0) {
        fprintf(f, TAB TAB "world[%d].directions[(%s)-1]=%s;\n",
            search_room(l), arg2, function_res);
    } else {
        fprintf(f, TAB TAB "world[search_room(%s)].directions[(%s)-1]=%s;\n",
            arg1, arg2, function_res);
    }


    free(arg1);
    free(arg2);
    return scanpos;
}
/** SWAP */
unsigned int action_swap(FILE *f, char *line, unsigned int scanpos)
{
    char *arg1;
    unsigned int l;
    start_function();
    scanpos=process_functions(line, scanpos);
    l=strlen(function_res);
    arg1=(char *) calloc(l+1,sizeof(char));
    if(arg1==NULL) {
        printf("Error allocating memory!\n");
        exit(1);
    }
    strcpy(arg1,function_res);

    start_function();
    scanpos=process_functions(line, scanpos);

    fprintf(f, TAB TAB "dummy=get_object_position(%s);\n",arg1);
    fprintf(f, TAB TAB
        "search_object_p(%s)->position=get_object_position(%s);\n",
        arg1,function_res);
    fprintf(f, TAB TAB "search_object_p(%s)->position=dummy;\n",
        function_res);
    free(arg1);
    return scanpos;
}
/** WAIT */
unsigned int action_wait(FILE *f, char *line, unsigned int scanpos)
{
    fprintf(f, TAB TAB "return;\n");
    return scanpos;
}
/** LF */
unsigned int action_lf(FILE *f, char *line, unsigned int scanpos)
{
    fprintf(f, TAB TAB "printnewline();\n");
    return scanpos;
}
/** OBJ */
unsigned int action_obj(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    if(compress_messages==true) {
        if(hardcoded_messages==false) {
            fprintf(f,TAB TAB "write_text(search_object_p(%s)->desc);\n",
                function_res);
        } else {
            fprintf(f,TAB TAB "show_message(search_object_p(%s)->desc);\n",
                function_res);
        }
    } else {
        fprintf(f, TAB TAB "writeln(search_object_p(%s)->desc);",
            function_res);
    }
    return scanpos;
}
/** WEAR */
unsigned int action_wear(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, TAB TAB "dummy=search_object(%s);\n",function_res);
    fprintf(f, TAB TAB "odummy=&obj[dummy];\n");

    fprintf(f, TAB TAB
        "if((odummy->attributes&ISWEREABLE)!=0&&(odummy->position==CARRIED||"
        "odummy->position==current_position)){\n");
    fprintf(f, TAB TAB TAB"odummy->position=WEARED;\n");
    fprintf(f, TAB TAB TAB "++counter[118];\n");
    fprintf(f, TAB TAB "} else if(odummy->position==WEARED) {\n");
    if(hardcoded_messages==false)
        fprintf(f, TAB TAB TAB "show_message(1019);\n");
    else
        fprintf(f, TAB TAB TAB "show_message(message1019);\n");
    fprintf(f, TAB TAB TAB "return;\n");
    fprintf(f, TAB TAB "} else {\n");
    if(hardcoded_messages==false)
        fprintf(f, TAB TAB TAB "show_message(1010);\n");
    else
        fprintf(f, TAB TAB TAB "show_message(message1010);\n");
    fprintf(f, TAB TAB TAB "return;\n");
    fprintf(f, TAB TAB "}\n");
    return scanpos;
}
/** UNWEAR */
unsigned int action_unwear(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    need_unwear=true;
    fprintf(f, TAB TAB "if(unwear(%s)) return;\n", function_res);
    return scanpos;
}
/** STRE */
unsigned int action_stre(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, TAB TAB "counter[122]=%s;\n",function_res);
    return scanpos;
}
/** RESTART */
unsigned int action_restart(FILE *f, char *line, unsigned int scanpos)
{
    fprintf(f, TAB TAB "if(are_you_sure())\n");
    fprintf(f, TAB TAB TAB "restart();\n");
    scanpos=action_wait(f, line, scanpos);
    return scanpos;
}
/** DIMENS */
unsigned int action_dimens(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, TAB TAB "counter[121]=%s;\n",function_res);
    return scanpos;
}
/** LOAD */
unsigned int action_load(FILE *f, char *line, unsigned int scanpos)
{
    fprintf(f, TAB TAB "LOAD;\n");
    return scanpos;
}
/** SAVE */
unsigned int action_save(FILE *f, char *line, unsigned int scanpos)
{
    fprintf(f, TAB TAB "SAVE;\n");
    return scanpos;
}

/** Main processing function. Exploits buffer.
*/
void process_aws(FILE *f, char *line)
{
    unsigned int scanpos=0;
    boolean atleastone=false;
    complete_shortcut=false;
    scanpos=get_token(line, scanpos);
    shortcuts=true;
    dont_issue_message=false;


    if(strcmp(token, "IF")!=0) {
        printf("Unrecognised start of aws condition %s instead of IF.\n"
            "Line [%s]", token, line);
        ++no_of_errors;
        return;
    }
    /* In some cases a NO1GT condition may imply that noun1>0, that can be
       skipped processing a NO1LT condition. */
    checked_noun1_greater_zero=false;
    fprintf(f,TAB "// %s\n",line);
    fprintf(f,TAB "if(");
    while(1) {  // DECISIONS
        scanpos=get_token(line, scanpos);
        if(strcmp(token,"AT")==0 || strcmp(token,"ROOMEQ")==0) {
            scanpos=decision_at(f, line, scanpos);
        } else if(strcmp(token,"NOTAT")==0) {
            scanpos=decision_notat(f, line, scanpos);
        }  else if(strcmp(token,"SET?")==0) {
            scanpos=decision_set(f, line, scanpos);
        } else if(strcmp(token,"RES?")==0) {
            scanpos=decision_res(f, line, scanpos);
        } else if(strcmp(token,"ROOMGT")==0) {
            scanpos=decision_roomgt(f, line, scanpos);
        } else if(strcmp(token,"ROOMLT")==0) {
            scanpos=decision_roomlt(f, line, scanpos);
        } else if(strcmp(token,"CARR")==0) {
            scanpos=decision_carr(f, line, scanpos);
        } else if(strcmp(token,"NOTIN")==0) {
            scanpos=decision_notin(f, line, scanpos);
        } else if(strcmp(token,"EQU?")==0) {
            scanpos=decision_equ(f, line, scanpos);
        } else if(strcmp(token,"NOTEQU?")==0) {
            scanpos=decision_notequ(f, line, scanpos);
        } else if(strcmp(token,"CTREQ")==0) {
            scanpos=decision_equ(f, line, scanpos);
        } else if(strcmp(token,"VERB")==0) {
            scanpos=decision_verb(f, line, scanpos);
        } else if(strcmp(token,"ADVEEQ")==0) {
            scanpos=decision_adve(f, line, scanpos);
        } else if(strcmp(token,"NOUN")==0) {
            scanpos=decision_noun(f, line, scanpos);
        } else if(strcmp(token,"ADVE")==0) {
            scanpos=decision_adve(f, line, scanpos);
        } else if(strcmp(token,"NO1EQ")==0) {
            scanpos=decision_no1eq(f, line, scanpos);
        } else if(strcmp(token,"AVAI")==0) {
            scanpos=decision_avai(f, line, scanpos);
        } else if(strcmp(token,"NOTAVAI")==0) {
            scanpos=decision_notavai(f, line, scanpos);
        } else if(strcmp(token,"NO2EQ")==0) {
            scanpos=decision_no2eq(f, line, scanpos);
        } else if(strcmp(token,"NOTCARR")==0) {
            scanpos=decision_notcarr(f, line, scanpos);
        } else if(strcmp(token,"NO1GT")==0) {
            scanpos=decision_no1gt(f, line, scanpos);
        } else if(strcmp(token,"NO1LT")==0) {
            scanpos=decision_no1lt(f, line, scanpos);
        } else if(strcmp(token,"NO2GT")==0) {
            scanpos=decision_no2gt(f, line, scanpos);
        } else if(strcmp(token,"NO2LT")==0) {
            scanpos=decision_no2lt(f, line, scanpos);
        } else if(strcmp(token,"CTRGT")==0) {
            scanpos=decision_ctrgt(f, line, scanpos);
        } else if(strcmp(token,"CTRLT")==0) {
            scanpos=decision_ctrlt(f, line, scanpos);
        } else if(strcmp(token,"IN")==0) {
            scanpos=decision_in(f, line, scanpos);
        } else if(strcmp(token,"HERE")==0) {
            scanpos=decision_here(f, line, scanpos);
        } else if(strcmp(token,"NOTHERE")==0) {
            scanpos=decision_nothere(f, line, scanpos);
        } else if(strcmp(token,"PROB")==0) {
            scanpos=decision_prob(f, line, scanpos);
        } else if(strcmp(token,"VBNOEQ")==0) {
            scanpos=decision_verb(f, line, scanpos);
        } else if(strcmp(token,"VBNOGT")==0) {
            scanpos=decision_vbnogt(f, line, scanpos);
        } else if(strcmp(token,"VBNOLT")==0) {
            scanpos=decision_vbnolt(f, line, scanpos);
        } else if(strcmp(token,"ISMOVABLE")==0) {
            scanpos=decision_ismovable(f, line, scanpos);
        } else if(strcmp(token,"ISNOTMOVABLE")==0) {
            scanpos=decision_isnotmovable(f, line, scanpos);
        } else if(strcmp(token,"ISWEARABLE")==0) {
            scanpos=decision_iswearable(f, line, scanpos);
        } else if(strcmp(token,"ISNOTWEARABLE")==0) {
            scanpos=decision_isnotwearable(f, line, scanpos);
        } else if(strcmp(token,"ISWEARING")==0) {
            scanpos=decision_iswearing(f, line, scanpos);
        } else if(strcmp(token,"ISNOTWEARING")==0) {
            scanpos=decision_isnotwearing(f, line, scanpos);
        } else if(strcmp(token,"ISCARRSOME")==0) {
            scanpos=decision_iscarrsome(f, line, scanpos);
        } else if(strcmp(token,"ISCARRNOTH")==0) {
            scanpos=decision_iscarrnoth(f, line, scanpos);
        } else if(strcmp(token,"ISWEARSOME")==0) {
            scanpos=decision_iswearsome(f, line, scanpos);
        } else if(strcmp(token,"ISWEARNOTH")==0) {
            scanpos=decision_iswearnoth(f, line, scanpos);
        } else if(strcmp(token,"OBJLOCEQ")==0) {
            scanpos=decision_objloceq(f, line, scanpos);
        } else if(strcmp(token,"OBJLOCGT")==0) {
            scanpos=decision_objlocgt(f, line, scanpos);
        } else if(strcmp(token,"OBJLOCLT")==0) {
            scanpos=decision_objloclt(f, line, scanpos);
        } else if(strcmp(token,"ACTOR")==0) {
            scanpos=decision_actor(f, line, scanpos);
        } else if(strcmp(token,"ACTOREQ")==0) {
            scanpos=decision_actor(f, line, scanpos);
        } else if(strcmp(token,"ACTORGT")==0) {
            scanpos=decision_actorgt(f, line, scanpos);
        } else if(strcmp(token,"ACTORLT")==0) {
            scanpos=decision_actorlt(f, line, scanpos);
        } else if(strcmp(token,"ADJE")==0) {
            scanpos=decision_adje(f, line, scanpos);
        } else if(strcmp(token,"ADJEEQ")==0) {
            scanpos=decision_adje(f, line, scanpos);
        } else if(strcmp(token,"ADJEGT")==0) {
            scanpos=decision_adjegt(f, line, scanpos);
        } else if(strcmp(token,"ADJELT")==0) {
            scanpos=decision_adjelt(f, line, scanpos);
        } else if(strcmp(token,"CONNEQ")==0) {
            scanpos=decision_conneq(f, line, scanpos);
        } else if(strcmp(token,"CONNGT")==0) {
            scanpos=decision_conngt(f, line, scanpos);
        } else if(strcmp(token,"CONNLT")==0) {
            scanpos=decision_connlt(f, line, scanpos);
        } else if(strcmp(token,"OR")==0) {
            //shortcuts=false;
            fprintf(f,"||");
        } else if(strcmp(token,"AND")==0) {
            shortcuts=true;
            // this is a trick to have the behaviour of AND check_position_marker_on it is in AWS.
            fprintf(f,") if("); // There, AND has the same priority of OR :-(
        } else if(strcmp(token,"THEN")==0) {
            // One needs to avoid if() C code for "IF THEN" conditions
            if(atleastone==false)
                fprintf(f,"1");
            break;
        } else {
            if(complete_shortcut==true)
                return;
            printf("Unrecognised decision %s in [%s].\n", token, line);
            ++no_of_errors;
            return;
        }
        atleastone=true;
    }


    fprintf(f,") {\n");  // ACTIONS
    while(1) {
        scanpos=get_token(line, scanpos);
        if(strcmp(token,"PRESSKEY")==0) {
            scanpos=action_presskey(f, line, scanpos);
        } else if(strcmp(token,"GOTO")==0) {
            scanpos=action_goto(f, line, scanpos);
            break;  /* Goto generates a return. Ignore what comes after. */
        } else if(strcmp(token,"ENDIF")==0) {
            break;
        } else if(strcmp(token,"SET")==0) {
            scanpos=action_set(f, line, scanpos);
        } else if(strcmp(token,"RESE")==0) {
            scanpos=action_rese(f, line, scanpos);
        } else if(strcmp(token,"CSET")==0) {
            scanpos=action_cset(f, line, scanpos);
        } else if(strcmp(token,"INCR")==0) {
            scanpos=action_incr(f, line, scanpos);
        } else if(strcmp(token,"DECR")==0) {
            scanpos=action_decr(f, line, scanpos);
        } else if(strcmp(token,"MESS")==0) {
            scanpos=action_mess(f, line, scanpos);
        } else if(strcmp(token,"MESSNOLF")==0) {
            scanpos=action_messnolf(f, line, scanpos);
        } else if(strcmp(token,"BRIN")==0) {
            scanpos=action_brin(f, line, scanpos);
        } else if(strcmp(token,"QUIT")==0) {
            scanpos=action_quit(f, line, scanpos);
            break;
        } else if(strcmp(token,"EXIT")==0) {
            scanpos=action_exit(f, line, scanpos);
            break;
        } else if(strcmp(token,"INVE")==0) {
            scanpos=action_inve(f, line, scanpos);
        } else if(strcmp(token,"NORD")==0) {
            scanpos=action_move(f, line, scanpos, 1);
        } else if(strcmp(token,"SUD")==0) {
            scanpos=action_move(f, line, scanpos, 2);
        } else if(strcmp(token,"EST")==0) {
            scanpos=action_move(f, line, scanpos, 3);
        } else if(strcmp(token,"OVEST")==0) {
            scanpos=action_move(f, line, scanpos, 4);
        } else if(strcmp(token,"ALTO")==0) {
            scanpos=action_move(f, line, scanpos, 5);
        } else if(strcmp(token,"BASSO")==0) {
            scanpos=action_move(f, line, scanpos, 6);
        } else if(strcmp(token,"LOOK")==0) {
            scanpos=action_look(f, line, scanpos);
        } else if(strcmp(token,"DROP")==0) {
            scanpos=action_drop(f, line, scanpos);
        } else if(strcmp(token,"TO")==0) {
            scanpos=action_to(f, line, scanpos);
        } else if(strcmp(token,"OKAY")==0) {
            scanpos=action_okay(f, line, scanpos);
        } else if(strcmp(token,"PRIN")==0) {
            scanpos=action_prin(f, line, scanpos);
        } else if(strcmp(token,"PRINNOLF")==0) {
            scanpos=action_prinnolf(f, line, scanpos);
        } else if(strcmp(token,"ADDC")==0) {
            scanpos=action_addc(f, line, scanpos);
        } else if(strcmp(token,"HOLD")==0) {
            scanpos=action_hold(f, line, scanpos);
        } else if(strcmp(token,"GET")==0) {
            scanpos=action_get(f, line, scanpos);
        } else if(strcmp(token,"SETCONN")==0) {
            scanpos=action_setconn(f, line, scanpos);
        } else if(strcmp(token,"SWAP")==0) {
            scanpos=action_swap(f, line, scanpos);
        } else if(strcmp(token,"WAIT")==0) {
            scanpos=action_wait(f, line, scanpos);
        } else if(strcmp(token,"GETALL")==0) {
            scanpos=action_getall(f, line, scanpos);
        } else if(strcmp(token,"DROPALL")==0) {
            scanpos=action_dropall(f, line, scanpos);
        } else if(strcmp(token,"RESETALL")==0) {        // Extension to AWS 3.0
            scanpos=action_resetall(f, line, scanpos);
        } else if(strcmp(token,"LF")==0) {
            scanpos=action_lf(f, line, scanpos);
        } else if(strcmp(token,"OBJ")==0) {
            scanpos=action_obj(f, line, scanpos);
        } else if(strcmp(token,"WEAR")==0) {
            scanpos=action_wear(f, line, scanpos);
        } else if(strcmp(token,"UNWEAR")==0) {
            scanpos=action_unwear(f, line, scanpos);
        } else if(strcmp(token,"STRE")==0) {
            scanpos=action_stre(f, line, scanpos);
        } else if(strcmp(token,"DIMENS")==0) {
            scanpos=action_dimens(f, line, scanpos);
        } else if(strcmp(token,"SENDALLROOM")==0) {
            scanpos=action_sendallroom(f, line, scanpos);
        } else if(strcmp(token,"PICT")==0) {
            // ??
            printf("PICT is not implemented\n");
        } else if(strcmp(token,"TEXT")==0) {
            // ??
            printf("TEXT is not implemented\n");
        } else if(strcmp(token,"RESTART")==0) {
            scanpos=action_restart(f, line, scanpos);
        } else if(strcmp(token,"LOAD")==0) {
            scanpos=action_load(f, line, scanpos);
        } else if(strcmp(token,"SAVE")==0) {
            scanpos=action_save(f, line, scanpos);
        } else if(strcmp(token,"RAMSAVE")==0) {
            // ??
            printf("RAMSAVE is not implemented\n");
        } else if(strcmp(token,"RAMLOAD")==0) {
            // ??
            printf("RAMLOAD is not implemented\n");
        } else if(strcmp(token,"SCRIPTON")==0) {
            // ??
            printf("SCRIPTON is not implemented\n");
        } else if(strcmp(token,"SCRIPTOFF")==0) {
            // ??
            printf("SCRIPTOFF is not implemented\n");
        } else {
            printf("Unrecognised action %s in [%s].\n", token, line);
            ++no_of_errors;
            return;
        }
    }
    fprintf(f, TAB "}\n\n");
}

/** Create the header of the output file.
*/
void output_header(FILE *of, int maxroomcode, int maxobjcode,
    int dsize, int rsize, int osize, char *name)
{
    fprintf(of,"/* File generated by aws2c */\n\n");
    //fprintf(of,"#include<stdio.h>\n");
    fprintf(of,"#include<stdlib.h>\n");

    FILE *hf=fopen(config_file?config_file:"config.h","w");
    if(hf==NULL) {
        fprintf(stderr, "Can't open configuration file.");
    }
    fprintf(hf,"/* File generated by aws2c */\n\n");
    fprintf(hf, "#define DSIZE %d\n",dsize);
    fprintf(hf, "#define RSIZE %d\n",rsize);
    fprintf(hf, "#define OSIZE %d\n",osize);
    fprintf(hf, "#define GAMEN \"%s\"\n",name);

    if(compress5bit_dict)
        fprintf(hf,"#define DICT5BIT\n");
    if(compress_hash_dict)
        fprintf(hf,"#define DICTHASH\n");
    if(use_6_directions==true) {
        fprintf(hf,"#define DIR_REDUCED\n");
    }
    if(dont_care_size_weight==true) {
        fprintf(hf,"#define NOSIZEWEIGHT\n");
    }
    if(no_obj_long_desc==true) {
        fprintf(hf,"#define NOLONGDESC\n");
    }
    if(maxroomcode<256) {
        fprintf(hf,"#define BYTEROOMCODE\n");
    }
    if(maxobjcode<256) {
        printf("maxobjcode: %d\n",maxobjcode);
        fprintf(hf,"#define BYTEOBJCODE\n");
    }
    if(adverb_as_byte) {
        fprintf(hf,"#define ADVERBTYPE unsigned char\n");
        printf("Adverbs in a unsigned char variable.\n");
    } else {
        fprintf(hf,"#define ADVERBTYPE unsigned int\n");
        printf("Adverbs in an unsigned int  variable.\n");
    }
    if(adjective_as_byte) {
        fprintf(hf,"#define ADJECTIVETYPE unsigned char\n");
        printf("Adjectives in a unsigned char variable.\n");
    } else {
        fprintf(hf,"#define ADJECTIVETYPE unsigned int\n");
        printf("Adjectives in an unsigned int  variable.\n");
    }
    if(actor_as_byte) {
        fprintf(hf,"#define ACTORTYPE unsigned char\n");
        printf("Actors in a unsigned char variable.\n");
    } else {
        fprintf(hf,"#define ACTORTYPE unsigned int\n");
        printf("Actors in an unsigned int  variable.\n");
    }

    if(use_adverbs) {
        printf("This game use adverbs.\n");
    } else {
        printf("This game does not use adverbs.\n");
        fprintf(hf,"#define NOADVERBS\n");
    }
    if(use_adjectives) {
        printf("This game uses adjectives.\n");
    } else {
        printf("This game does not use adjectives.\n");
        fprintf(hf, "#define NOADJECTIVES\n");
    }
    fprintf(hf,"#define AVOID_SDESC\n");
    fclose(hf);
    fprintf(of,"#include\"%s\"\n\n", config_file?config_file:"config.h");
    fprintf(of,"#include\"aws.h\"\n\n");
    fprintf(of,"#include\"inout.h\"\n");
    fprintf(of,"#include\"systemd.h\"\n\n");
    fprintf(of,"extern unsigned int verb;\nextern unsigned int noun1;\nextern unsigned int noun2;\n"
        "extern ADVERBTYPE adve;\nextern ACTORTYPE actor;\nextern ADJECTIVETYPE adjective;\n");
    fprintf(of, "unsigned int dummy;\n");
    fprintf(of, "EFFSHORTINDEX cdummy;\n");
    fprintf(of, "#define CARRIED 1500\n");
    fprintf(of, "#define WEARED 1600\n");
    fprintf(of, "room *cr;\n");
    fprintf(of,"\n");

}

void output_optional_func(FILE *of, int max_room_code)
{
    if(need_searchw) {
        fprintf(of,"char *nonestr=\"\";\n");
        fprintf(of,"char *searchw(unsigned int w) FASTCALL\n{\n");
        fprintf(of, TAB "int i;\n");
        fprintf(of, TAB "for(i=0;i<DSIZE;++i)\n");
        fprintf(of, TAB TAB "if(w==dictionary[i].code)\n");
        fprintf(of, TAB TAB TAB "return dictionary[i].w;\n");
        fprintf(of, TAB "return nonestr;\n");
        fprintf(of, "}\n");
    }
    if(need_unwear) {
        fprintf(of, "boolean unwear(unsigned int o) FASTCALL\n{\n");
        fprintf(of, TAB "dummy=search_object(o);\n");
        fprintf(of, TAB "odummy=&obj[dummy];\n");

        fprintf(of, TAB "if(odummy->position==WEARED){\n");
        fprintf(of, TAB TAB "odummy->position=CARRIED;\n");
        fprintf(of, TAB TAB "--counter[118];\n");
        fprintf(of, TAB "} else {\n");
        if(hardcoded_messages==false)
            fprintf(of, TAB TAB "show_message(1010);\n");
        else
            fprintf(of, TAB TAB "show_message(message1010);\n");
        fprintf(of, TAB TAB "return true;\n");
        fprintf(of, TAB "}\n");
        fprintf(of, TAB "return false;\n");
        fprintf(of, "}\n");
    }
    if(need_cv) {
        /* Check for a verb */
        fprintf(of, "#ifdef CV_IS_A_FUNCTION\n");
        fprintf(of, "boolean cv(unsigned char v) FASTCALL\n");
        fprintf(of, "{\n");
        fprintf(of, "    return verb==v;\n");
        fprintf(of, "}\n");
        fprintf(of, "#endif\n");
    }
    if(need_vov) {
        /* Check among two verbs */

        fprintf(of, "boolean vov(unsigned int v1, unsigned int v2)\n");
        fprintf(of, "{\n");
        fprintf(of, "    return verb==v1||verb==v2;\n");
        fprintf(of, "}\n");
    }
    if(need_vovn) {
        /* Check among two verbs AND a name */
        fprintf(of,
            "boolean vovn(unsigned int v1, unsigned int v2, unsigned int n)\n");
        fprintf(of, "{\n");
        if(need_vov)
            fprintf(of, "    return vov(v1,v2)&&noun1==n;\n");
        else
            fprintf(of, "    return (verb==v1||verb==v2)&&noun1==n;\n");
        fprintf(of, "}\n");
        fprintf(of, "boolean vovn100_0(EFFSHORTINDEX n) FASTCALL\n");
        fprintf(of, "{\n");
        fprintf(of, "    return vovn(100,0,n);\n");
        fprintf(of, "}\n");
    }
    if(need_non1) {
        /* Check among two nouns1 */
        fprintf(of, "boolean non1(unsigned int n1, unsigned int n2)\n");
        fprintf(of, "{\n");
        fprintf(of, "    return noun1==n1||noun1==n2;\n");
        fprintf(of, "}\n");
    }
    if(need_cpi) {
        /* Check if position is equal to a certain value */
        fprintf(of, "boolean cpi(unsigned char p) FASTCALL\n");
        fprintf(of, "{\n");
        fprintf(of, "    return current_position==p;\n");
        fprintf(of, "}\n");
    }
    if(need_ok) {
        fprintf(of, "void ok(void)\n");
        fprintf(of, "{\n");
        if(hardcoded_messages==false)
            fprintf(of, TAB TAB "show_message(1000);\n");
        else
            fprintf(of, TAB TAB "show_message(message1000);\n");
        fprintf(of, "}\n");
    }
    if(need_cvn) {
        /* Check for a name and noun */
        fprintf(of, "boolean check_verb_noun(unsigned int v, unsigned int n)\n");
        fprintf(of, "{\n");

        fprintf(of, "    return verb==v&&noun1==n;\n");
        fprintf(of, "}\n");
        fprintf(of, "boolean cvn70(EFFSHORTINDEX n) FASTCALL\n");
        fprintf(of, "{\n");
        fprintf(of, "    return check_verb_noun(70,n);\n");
        fprintf(of, "}\n");
    }
    if(need_check_verb_actor) {
        /* Check for a verb and actor */
        fprintf(of, "boolean check_verb_actor(unsigned int v, ACTORTYPE n)\n");
        fprintf(of, "{\n");
        fprintf(of, "    return verb==v&&actor==n;\n");
        fprintf(of, "}\n");
        fprintf(of, "boolean check_verb75_actor(EFFSHORTINDEX n) FASTCALL\n");
        fprintf(of, "{\n");
        fprintf(of, "    return check_verb_actor(75,n);\n");
        fprintf(of, "}\n");
        fprintf(of, "boolean check_verb70_actor(EFFSHORTINDEX n) FASTCALL\n");
        fprintf(of, "{\n");
        fprintf(of, "    return check_verb_actor(70,n);\n");
        fprintf(of, "}\n");
        fprintf(of, "boolean check_verb70_actor_available(EFFSHORTINDEX n)"
            " FASTCALL\n");
        fprintf(of, "{\n");
        fprintf(of, "    return check_verb_actor(70,n)&&"
            "object_is_available(n);\n");
        fprintf(of, "}\n");
        fprintf(of, "boolean check_verb75_actor_available(EFFSHORTINDEX n)"
            " FASTCALL\n");
        fprintf(of, "{\n");
        fprintf(of, "    return check_verb_actor(75,n)&&"
            "object_is_available(n);\n");
        fprintf(of, "}\n");
    }
    if(need_cvna) {
        /* If a name and a noun and avai conditions are given */
        fprintf(of, "boolean cvna(unsigned int v, unsigned int n, "
            "unsigned int o)\n");
        fprintf(of, "{\n    dummy=get_object_position(o);\n");
        if(need_cvn)
            fprintf(of, "    return check_verb_noun(v,n)&&"
                "(dummy==current_position||dummy==CARRIED);\n");
        else
            fprintf(of, "    return verb==v&&noun1==n&&"
                "(dummy==current_position||dummy==CARRIED);\n");
        fprintf(of, "}\n");
        fprintf(of,"boolean cvna70(EFFSHORTINDEX n, EFFSHORTINDEX o)\n");
        fprintf(of, "{\n");
        fprintf(of, "   return cvna(70,n,o);\n");
        fprintf(of, "}\n");
        fprintf(of,"boolean cvna70neq(EFFSHORTINDEX n) FASTCALL\n");
        fprintf(of, "{\n");
        fprintf(of, "   return cvna70(n,n);\n");
        fprintf(of, "}\n");

    }
    if(need_sendallroom) {
        fprintf(of, "void sendallroom(unsigned int s) FASTCALL\n{\n");
        fprintf(of, TAB "for(dummy=0; dummy<OSIZE;++dummy){\n");
        fprintf(of, TAB TAB "odummy=&obj[dummy];\n");
        fprintf(of, TAB TAB
            "if(odummy->position==CARRIED) {\n");
        fprintf(of, TAB TAB TAB
            "odummy->position=s;\n");
        if(dont_care_size_weight==false) {
            fprintf(of, TAB TAB TAB "--counter[119];\n");
            fprintf(of, TAB TAB TAB "counter[120]-=odummy->weight;\n");
            fprintf(of, TAB TAB TAB "counter[124]-=odummy->size;\n");
        }
        fprintf(of, TAB TAB "}\n");
        fprintf(of, TAB "}\n");
        fprintf(of, "}\n");
    }
    if(need_hold) {
        fprintf(of, "void hold(unsigned int p) FASTCALL\n{\n");
        fprintf(of, TAB "for(dummy=0; dummy<p; ++dummy) {wait1s();}\n");
        fprintf(of, "}\n");
    }
    if(need_iscarrsome) {
        fprintf(of, "char iscarrsome(void)\n{\n");
        fprintf(of, TAB "for(dummy=0; dummy<OSIZE; ++dummy)\n");
        fprintf(of, TAB TAB "if(obj[dummy]==WEARED) return true;\n");
        fprintf(of, TAB "return false;\n");
        fprintf(of, "}\n");
    }
    if(need_iswearsome) {
        fprintf(of, "char iswearsome(void)\n{\n");
        fprintf(of, TAB "for(dummy=0; dummy<OSIZE; ++dummy)\n");
        fprintf(of, TAB TAB "if(obj[dummy]==CARRIED) return true;\n");
        fprintf(of, TAB "return false;\n");
        fprintf(of, "}\n");
    }
/*    if(need_checkexit) {
        fprintf(of, "void checkexit(void)\n{\n");
        if(compress_messages==true) {
            if(hardcoded_messages==true) {
                fprintf(of, TAB "show_message(exitrestart);\n");
            } else {
                fprintf(of, TAB "write_textsl(exitrestart);\n");
            }
        } else {
            fprintf(of, TAB "writeln(\"%s\");\n",EXITRESTART);
        }
        fprintf(of, TAB "GETS(playerInput,BUFFERSIZE);\n");
        fprintf(of, TAB "if(playerInput[0]=='E' || playerInput[0]=='e'){\n");
        fprintf(of, TAB TAB "leave(); exit(0);\n");
        fprintf(of, TAB "}\n");
        fprintf(of, "}\n");
    }*/
    if(need_amsm) {
        if(hardcoded_messages==false) {
            fprintf(of,"void amsm(");
            if(max_room_code>255)
                fprintf(of,"unsigned int");
            else
                fprintf(of,"EFFSHORTINDEX");
            fprintf(of," p, EFFSHORTINDEX c, boolean v, unsigned int m)\n");

        } else {
            fprintf(of,"void amsm(");
            if(max_room_code>255)
                fprintf(of,"unsigned int");
            else
                fprintf(of,"EFFSHORTINDEX");
            fprintf(of," p, EFFSHORTINDEX c, boolean v, char *m)\n");
        }

        fprintf(of, "{\n");
        fprintf(of,
            "    if(current_position==p&&marker[c]==v) show_message(m);\n");
        fprintf(of, "}\n");
    }
    if(need_as) {
        fprintf(of, "char check_position_marker_on(unsigned int p,"
            "unsigned char f)\n{\n");
        fprintf(of, "    return current_position==p&&marker[f];\n");
        fprintf(of, "}\n");
    }
    if(need_ar) {
        fprintf(of, "char check_position_marker_off(unsigned int p, "
            "unsigned char f)\n{\n");
        fprintf(of, "    return current_position==p&&!marker[f];\n");
        fprintf(of, "}\n");
    }
    if(need_ams) {
        if(hardcoded_messages==false) {
            fprintf(of,"unsigned char ams(unsigned char  v, unsigned char n, "
                "unsigned int m)\n");

        } else {
            fprintf(of,"unsigned char ams(unsigned char  v, unsigned char n, "
                "char* m)\n");;
        }

        fprintf(of, "{\n");
        fprintf(of,
            "    if(verb==v && noun1==n) { show_message(m); return 1;}\n");
        fprintf(of, "    return 0;\n");
        fprintf(of, "}\n");
    }
}

void output_utility_func(FILE *of, info *header, int rsize, int osize,
    int max_room_code)
{
    fprintf(of,"room_code current_position;\n");
    fprintf(of,"room_code next_position;\n");
    fprintf(of,"boolean retv;\n");
    fprintf(of,"extern EFFSHORTINDEX ls;\n");
    fprintf(of,"extern char playerInput[];\n");

    fprintf(of,"boolean marker[129];\n");
    fprintf(of,"int counter[129];\n");

    fprintf(of,"object *odummy;\n\n");
    /* Introduces the prototype here, functions will be included only if
       necessary, after having analyzed the file. */
    fprintf(of,"char *searchw(unsigned int w) FASTCALL;\n");
    fprintf(of, "boolean unwear(unsigned int o) FASTCALL;\n");

    fprintf(of,"void printnewline(void)\n{\n");
    fprintf(of,TAB "writesameln(\"\\n\");\n}\n\n");
    fprintf(of,"void printspace(void)\n{\n");
    fprintf(of,TAB "writesameln(\" \");\n}\n\n");


    if(osize>255) {
        fprintf(of,"unsigned int search_object(unsigned int o) FASTCALL\n{\n");
        fprintf(of,TAB "unsigned int idx;\n");

    } else {
        fprintf(of,"EFFSHORTINDEX search_object(unsigned int o) FASTCALL\n{\n");
        fprintf(of,TAB "EFFSHORTINDEX idx;\n");
    }
    fprintf(of,TAB "for(idx=0; idx<OSIZE;++idx)\n");
    fprintf(of,TAB TAB "if(obj[idx].code==o)\n");
    fprintf(of,TAB TAB TAB "return idx;\n");
    fprintf(of,TAB "return 0;\n}\n\n");

    fprintf(of,"object *search_object_p(unsigned int o) FASTCALL\n");
    fprintf(of,"{\n");
    fprintf(of,TAB "return &obj[search_object(o)];\n");
    fprintf(of,"}\n");

    if(max_room_code>255) {
        fprintf(of,"unsigned int search_room(unsigned int r) FASTCALL\n{\n");
        /* Local variable replaces global one. */
        fprintf(of,TAB "unsigned int cdummy;\n");
    } else {
        fprintf(of,"EFFSHORTINDEX search_room(EFFSHORTINDEX r) FASTCALL\n{\n");
    }

    fprintf(of,TAB "for(cdummy=0; cdummy<RSIZE;++cdummy)\n");
    fprintf(of,TAB TAB "if(world[cdummy].code==r)\n");
    fprintf(of,TAB TAB TAB "return cdummy;\n");
    fprintf(of,TAB "return 0;\n}\n\n");

    fprintf(of,"void restart(void)\n{\n");
    if(max_room_code>255||osize>255)
        fprintf(of,TAB "unsigned int j;\n");
    else
        fprintf(of,TAB "EFFSHORTINDEX j;\n");
    fprintf(of,TAB "for(cdummy=1;cdummy<129;++cdummy){\n");
    fprintf(of,TAB TAB "marker[cdummy]=0;\n");
    fprintf(of,TAB TAB "counter[cdummy]=0;\n");
    fprintf(of,TAB "}\n");
    fprintf(of,TAB "for(j=0; j<RSIZE;++j)\n");
    fprintf(of,TAB TAB "for(cdummy=0; cdummy<NDIR;++cdummy)\n");
    fprintf(of,TAB TAB TAB
        "world[j].directions[cdummy]=original_connections[j][cdummy];\n");

    fprintf(of, TAB "marker[124]=true;\n");
    if(!dont_use_light) fprintf(of, TAB "marker[121]=true;\n");
    if(header->maxcarryingw==0) {
        header->maxcarryingw=10000;
    }
    if(header->maxcarryings==0) {
        header->maxcarryings=10000;
    }
    fprintf(of, TAB "next_position=%d;\n",header->startroom);
    if(!dont_care_size_weight) {
        fprintf(of, TAB "counter[121]=%d;\n",header->maxcarryings);
        fprintf(of, TAB "counter[122]=%d;\n",header->maxcarryingw);
    }
    fprintf(of, TAB "for(j=0; j<OSIZE;++j)\n");
    fprintf(of, TAB TAB "obj[j].position=original_position[j];\n");
    fprintf(of, "}\n\n");

    if(hardcoded_messages==false) {
        if(compress_messages==true) {
            fprintf(of,"void write_textsl(char *m) FASTCALL\n{\n");
            fprintf(of,TAB "char r;\n");
            fprintf(of,TAB "cpointer=0;\n");
            fprintf(of,TAB "bpointer=0;\n");
            fprintf(of,TAB "compressed=m;\n");
            fprintf(of,TAB "do {\n");
            fprintf(of,TAB TAB "r=decode();\n");
            fprintf(of,TAB TAB "writesameln(decompress_b);\n");
            fprintf(of,TAB "} while(r);\n");
            fprintf(of, "}\n");
            fprintf(of,"void write_text(char *m)\n{\n");
            fprintf(of,TAB "write_textsl(m);\n");
            fprintf(of,TAB "printnewline();\n");
            fprintf(of, "}\n");
        }
        fprintf(of,"void show_messagenlf(unsigned int m) FASTCALL\n{\n");
        fprintf(of,TAB "unsigned int i;\n");
        fprintf(of,TAB "for(i=0; i<MSIZE;++i)\n");
        fprintf(of,TAB TAB "if(msg[i].code==m){\n");
        if(compress_messages==true) {
            fprintf(of,TAB TAB TAB "write_textsl(msg[i].txt);\n");
        } else {
            fprintf(of,TAB TAB TAB "writesameln(msg[i].txt);\n");
        }
        fprintf(of, TAB TAB TAB "break;\n");
        fprintf(of, TAB TAB "}\n");
        fprintf(of, "}\n\n");

        fprintf(of,"void show_message(unsigned int m) FASTCALL\n{\n");
        fprintf(of,TAB "show_messagenlf(m);\n");
        fprintf(of,TAB "printnewline();\n");
        fprintf(of, "}\n\n");
    } else {
        fprintf(of,"void show_messagenlf(const char *m) FASTCALL\n{\n");
        if(compress_messages==true) {
            fprintf(of,TAB "boolean o;\n");
            fprintf(of,TAB "cpointer=0;\n");
            fprintf(of,TAB "bpointer=0;\n");
            fprintf(of,TAB "compressed=(char *)m;\n");
            fprintf(of,TAB "do{\n");
            fprintf(of,TAB TAB "o=decode();\n");
            fprintf(of,TAB TAB "writesameln(decompress_b);\n");
            fprintf(of,TAB "} while(o);\n");

        } else {
            fprintf(of,TAB "writesameln(m);\n");
        }
        fprintf(of, "}\n\n");

        fprintf(of,"void show_message(const char *m) FASTCALL\n{\n");
        fprintf(of,TAB "show_messagenlf(m);\n");
        fprintf(of,TAB "printnewline();\n");
        fprintf(of, "}\n\n");
    }

    fprintf(of, "boolean are_you_sure(void)\n{\n");
    if(compress_messages==true) {
        if(hardcoded_messages==true) {
            fprintf(of, TAB "show_message(areyousure);\n");
        } else {
            fprintf(of, TAB "write_textsl(areyousure);\n");
        }
    } else {
        fprintf(of, TAB "writeln(\"%s\");\n",AREYOUSURE);
    }
    fprintf(of, TAB "GETS(playerInput,BUFFERSIZE);\n");
    fprintf(of, TAB "if(playerInput[0]=='Y' || playerInput[0]=='y') {\n");
    fprintf(of, TAB TAB "return 1;\n");
    fprintf(of, TAB "}\n");
    fprintf(of, TAB "return 0;\n");
    fprintf(of, "}\n\n");


    fprintf(of,"#define inventory()\\\n{\\\n");
    fprintf(of,TAB "boolean gs=false;\\\n");
    if(osize>255) {
        /* Local variable replaces a global one. */
        fprintf(of,TAB "unsigned int cdummy;\\\n");
    }


    if(hardcoded_messages==false)
        fprintf(of,TAB "show_message(1032);\\\n");
    else
        fprintf(of,TAB "show_message(message1032);\\\n");
    fprintf(of,TAB "for(cdummy = 0; cdummy<OSIZE; ++cdummy) {\\\n");
    fprintf(of,TAB TAB "dummy=obj[cdummy].position;\\\n");
    fprintf(of,TAB TAB "if(dummy==CARRIED||dummy==WEARED) {\\\n");
    fprintf(of,TAB TAB TAB "gs=true;\\\n");
    fprintf(of,TAB TAB TAB "evidence2();\\\n");
    if(compress_messages==true) {
        if(hardcoded_messages==false) {
            fprintf(of,TAB TAB TAB "write_textsl(obj[cdummy].desc);\\\n");
        } else {
            fprintf(of,TAB TAB TAB "show_messagenlf(obj[cdummy].desc);\\\n");
        }
    } else {
        fprintf(of,TAB TAB TAB "writeln(obj[cdummy].desc);\\\n");
    }
    fprintf(of,TAB TAB TAB "normaltxt();\\\n");
    fprintf(of,TAB TAB TAB "if(dummy==WEARED){\\\n");
    fprintf(of,TAB TAB TAB TAB "printspace();\\\n");
    if(hardcoded_messages==false)
        fprintf(of,TAB TAB TAB TAB "show_messagenlf(1018);\\\n");
    else
        fprintf(of,TAB TAB TAB TAB "show_messagenlf(message1018);\\\n");
    fprintf(of,TAB TAB TAB "}\\\n");
    fprintf(of,TAB TAB TAB "printnewline();\\\n");
    fprintf(of,TAB TAB "}\\\n");
    fprintf(of,TAB "}\\\n");

    if(hardcoded_messages==false)
        fprintf(of,TAB "if(gs==false) show_message(1033);\\\n}\n\n");
    else
        fprintf(of,TAB "if(gs==false) show_message(message1033);\\\n}\n\n");

    fprintf(of, "void move(EFFSHORTINDEX dir) FASTCALL\n");
    fprintf(of, "{\n");
    if(max_room_code>255)
        fprintf(of, TAB "unsigned int p;\n");
    else
        fprintf(of, TAB "EFFSHORTINDEX p;\n");
    fprintf(of, TAB
        "p=cr->directions[dir];\n");
    fprintf(of, TAB
        "if(p) {\n");
    fprintf(of, TAB TAB "next_position=p;\n");
    fprintf(of, TAB TAB "marker[120]=false;\n");
    fprintf(of, TAB "} else \n");
    if(hardcoded_messages==false)
        fprintf(of, TAB TAB "show_message(1008);\n");
    else
        fprintf(of, TAB TAB "show_message(message1008);\n");
    fprintf(of, "\n}\n\n");

    fprintf(of, "boolean get(unsigned int o) FASTCALL\\\n");
    fprintf(of, "{\n");
    fprintf(of, TAB "odummy=search_object_p(o);\n");

    fprintf(of, TAB "if(odummy->position!=current_position) {\n");
    if(hardcoded_messages==false)
        fprintf(of, TAB TAB "show_message(1006);\n");
    else
        fprintf(of, TAB TAB "show_message(message1006);\n");
    /* Euh... should not be isnotmovable==true here??? */
    fprintf(of, TAB "} else if((odummy->attributes&ISNOTMOVABLE)==0) {\n");
    if(hardcoded_messages==false)
        fprintf(of, TAB TAB "show_message(1005);\n");
    else
        fprintf(of, TAB TAB "show_message(message1005);\n");
    if(dont_care_size_weight==false) {
        fprintf(of, TAB
            "} else if(counter[120]+odummy->weight>counter[122]){ \n");
        if(hardcoded_messages==false)
            fprintf(of, TAB TAB "show_message(1003);\n");
        else
            fprintf(of, TAB TAB "show_message(message1003);\n");
        fprintf(of, TAB
            "} else if(counter[124]+odummy->size>counter[121]) {\n");
        if(hardcoded_messages==false)
            fprintf(of, TAB TAB "show_message(1004);\n");
        else
            fprintf(of, TAB TAB "show_message(message1004);\n");
    }
    fprintf(of, TAB "} else {\n");
    fprintf(of, TAB TAB "odummy->position=CARRIED;\n");
    if(dont_care_size_weight==false) {
        fprintf(of, TAB TAB "++counter[119];\n");
        fprintf(of, TAB TAB "counter[120]+=odummy->weight;\n");
        fprintf(of, TAB TAB "counter[124]+=odummy->size;\n");
    }
    fprintf(of, TAB TAB "return false;\n");
    fprintf(of, TAB "}\n");
    fprintf(of, TAB "return true;\n");
    fprintf(of, "}\n");

    /* Check among two verbs */
    fprintf(of, "boolean vov(unsigned int v1, unsigned int v2);\n");

    /* Check among two verbs AND a name */
    fprintf(of,
        "boolean vovn(unsigned int v1, unsigned int v2, unsigned int n);\n");
    fprintf(of,"boolean vovn100_0(EFFSHORTINDEX n) FASTCALL;\n");

    /* Check among two nouns1 */
    fprintf(of, "boolean non1(unsigned int n1, unsigned int n2);\n");

    /* Check if the current position is equal to p*/
    //fprintf(of, "boolean cpi(unsigned char p);\n");

    fprintf(of, "void ok(void);\n");

    /* Check for a verb and noun */
    fprintf(of, "boolean check_verb_noun(unsigned int v, unsigned int n);\n");
    fprintf(of, "boolean cvn70(EFFSHORTINDEX n) FASTCALL;\n");

    /* Check for a name and an actor */
    fprintf(of, "boolean check_verb_actor(unsigned int v, ACTORTYPE n);\n");
    fprintf(of, "boolean check_verb75_actor(EFFSHORTINDEX n) FASTCALL;\n");
    fprintf(of, "boolean check_verb75_actor_available(EFFSHORTINDEX n) "
        "FASTCALL;\n");
    fprintf(of, "boolean check_verb70_actor(EFFSHORTINDEX n) FASTCALL;\n");
    fprintf(of, "boolean check_verb70_actor_available(EFFSHORTINDEX n) "
        "FASTCALL;\n");



    if(hardcoded_messages==false) {
        fprintf(of,"unsigned char ams(unsigned char  v, unsigned char n, "
            "unsigned int m);\n");

    } else {
        fprintf(of,"unsigned char ams(unsigned char  v, unsigned char n, "
            "char* m);\n");;
    }

    /* Check for a verb */
    fprintf(of, "#ifdef CV_IS_A_FUNCTION\n");
    fprintf(of, "    boolean cv(unsigned char v) FASTCALL;\n");
    fprintf(of, "#else\n");
    fprintf(of, "    #define cv(v) verb==(v)\n");
    fprintf(of, "#endif\n");

    /* Send all objects to a room in particular */
    fprintf(of, "void sendallroom(unsigned int s) FASTCALL;\n");

    fprintf(of, "#ifdef CV_IS_A_FUNCTION\n");
     /* Get current position of an object */
    fprintf(of, TAB "unsigned int get_object_position(obj_code c) FASTCALL\n");
    fprintf(of, TAB "{\n");
    fprintf(of, TAB "    return search_object_p(c)->position;\n");
    fprintf(of, TAB "}\n");
    /* Check if an object is here */
    fprintf(of, TAB "boolean object_is_here(obj_code c) FASTCALL\n");
    fprintf(of, TAB "{\n");
    fprintf(of,
        TAB "    return get_object_position(c)==current_position;\n");
    fprintf(of, TAB "}\n");
        /* Check if an object is carried */
    fprintf(of, TAB "boolean object_is_carried(obj_code c) FASTCALL\n");
    fprintf(of, TAB "{\n");
    fprintf(of,
        TAB "    return get_object_position(c)==CARRIED;\n");
    fprintf(of, TAB "}\n");
    fprintf(of, "#else\n");
    fprintf(of,
        TAB "#define object_is_here(c) (search_object_p(c)->position"
            "==current_position)\n");
    fprintf(of,
        TAB "#define get_object_position(c) search_object_p(c)->position\n");
    fprintf(of,
        TAB "#define object_is_carried(c) "
            "(search_object_p(c)->position==CARRIED)\n");
    fprintf(of, "#endif\n");

    /* Check if an object is available */
    fprintf(of, "boolean object_is_available(obj_code c) FASTCALL\n");
    fprintf(of, "{\n");
    fprintf(of,
        "    return object_is_here(c)||object_is_carried(c);\n");
    fprintf(of, "}\n");
    /* Set current position of an object */
    fprintf(of, "void set_object_position(obj_code c, int pos)\n");
    fprintf(of, "{\n");
    fprintf(of, "    search_object_p(c)->position=pos;\n");
    fprintf(of, "}\n");
    fprintf(of, "void set_object_position0(obj_code c) FASTCALL\n");
    fprintf(of, "{\n");
    fprintf(of, "    set_object_position(c,0);\n");
    fprintf(of, "}\n");
    fprintf(of, "void set_object_positionC(obj_code c) FASTCALL\n");
    fprintf(of, "{\n");
    fprintf(of, "    set_object_position(c,CARRIED);\n");
    fprintf(of, "}\n");

    /* Bring here an object */
    fprintf(of, "void bring_object_here(obj_code c) FASTCALL\n");
    fprintf(of, "{\n");
    fprintf(of, "    set_object_position(c,current_position);\n");
    fprintf(of, "}\n");

     /* Check for a position and marker */

    if(hardcoded_messages==false) {
        fprintf(of,"void amsm(");
        if(max_room_code>255)
            fprintf(of,"unsigned int");
        else
            fprintf(of,"EFFSHORTINDEX");
        fprintf(of," p, EFFSHORTINDEX c, boolean v, unsigned int m);\n");

    } else {
        fprintf(of,"void amsm(");
        if(max_room_code>255)
            fprintf(of,"unsigned int");
        else
            fprintf(of,"EFFSHORTINDEX");
        fprintf(of," p, EFFSHORTINDEX c, boolean v, char *m);\n");
    }

    /* If a name and a noun and avai conditions are given */
    fprintf(of,
        "boolean cvna(unsigned int v, unsigned int n, unsigned int o);\n");
    fprintf(of,
        "boolean cvna70(EFFSHORTINDEX n, EFFSHORTINDEX o);\n");
    fprintf(of,
        "boolean cvna70neq(EFFSHORTINDEX n) FASTCALL;\n");

    //if(drop_as_function) {
    fprintf(of, "boolean drop(unsigned int o) FASTCALL\n{\n");
    /*} else {
        fprintf(of, "#define drop(o) \\\n{\\\n");
    }*/
    fprintf(of, TAB  "odummy=search_object_p(o);\\\n");

    fprintf(of, TAB  "if(odummy->position==CARRIED){\\\n");
    fprintf(of, TAB TAB "odummy->position=current_position;\\\n");
    if(dont_care_size_weight==false) {
        fprintf(of, TAB TAB "--counter[119];\\\n");
        fprintf(of, TAB TAB "counter[120]-=odummy->weight;\\\n");
        fprintf(of, TAB TAB "counter[124]-=odummy->size;\\\n");
    }
    fprintf(of, TAB "} else {\\\n");
    if(hardcoded_messages==false)
        fprintf(of, TAB TAB "show_message(1007);\\\n");
    else
        fprintf(of, TAB TAB "show_message(message1007);\\\n");
    fprintf(of, TAB TAB "return true;\\\n");
    fprintf(of, TAB "}\\\n");
    fprintf(of, TAB "return false;\\\n");
    fprintf(of, "}\n\n");


    if(jump_as_function) {
        fprintf(of, "void jump(");
        if(max_room_code>255)
            fprintf(of, "unsigned int");
        else
            fprintf(of, "EFFSHORTINDEX");
        fprintf(of, " p) FASTCALL\n{\n");
        fprintf(of, TAB "next_position=p;\n");
        fprintf(of, TAB "marker[120]=false;\n");
        fprintf(of, "}\n\n");
    } else {
        fprintf(of, "#define jump(p)\\\n");
        fprintf(of, "{\\\n");
        fprintf(of, TAB "next_position=p;\\\n");
        fprintf(of, TAB "marker[120]=false;\\\n");
        fprintf(of, "}\n\n");
    }

    fprintf(of, "void hold(unsigned int p) FASTCALL;\n");
    fprintf(of, "char iscarrsome(void);\n");
    fprintf(of, "char iswearsome(void);\n");
    //fprintf(of, "void checkexit(void);\n");
    // I decided that a macro is better, check_position_marker_on it is called only once.
    fprintf(of, "#define checkexit()\\\n{\\\n");
    if(compress_messages==true) {
        if(hardcoded_messages==true) {
            fprintf(of, TAB "show_message(exitrestart);\\\n");
        } else {
            fprintf(of, TAB "write_textsl(exitrestart);\\\n");
        }
    } else {
        fprintf(of, TAB "writeln(\"%s\");\\\n",EXITRESTART);
    }
    fprintf(of, TAB "GETS(playerInput,BUFFERSIZE);\\\n");
    fprintf(of, TAB "if(playerInput[0]=='E' || playerInput[0]=='e'){\\\n");
    fprintf(of, TAB TAB "leave(); exit(0);\\\n");
    fprintf(of, TAB "}\\\n");
    fprintf(of, "}\n");

    fprintf(of, "char check_position_marker_on(unsigned int p, unsigned char f);\n");
    fprintf(of, "char check_position_marker_off(unsigned int p, unsigned char f);\n");

}

/* Compress a word using a 5-bit encoding:

A - 1
B - 2
C - 3
...

The compressed word will substitute the original one in the buffer.
*/
void compress_5bit(char *buffer)
{
    char *pcomp=buffer;
    unsigned int shift=0;
    unsigned int c;

    while((c=*buffer)!='\0') {
        *(buffer++)='\0';
        if(shift>7)
            shift-=8;
        c=toupper(c);
        c=(c-'@')&0x1F;
        c<<=shift;
        *pcomp |=c&0x00FF;
        if(shift>=3)
            *(++pcomp)=(c&0xFF00)>>8;

        shift+=5;
    }
}

/*  Store the dictionary check_position_marker_on a 3-byte hash code for each word.
    The result is a 3-character string that substitutes the original one.
*/
void compress_hash(char *buffer)
{
    char *pcomp=buffer;
    unsigned int i=0, j=0;
    unsigned char c;
    #define N 3
    while((c=buffer[j++])!='\0') {
        c^=(0xFF -i);
        if(j>N)
            buffer[i]^=c;
        if(++i>N-1) i=0;
    }
    if(j<2) buffer[1]=0;
    if(j<3) buffer[2]=0;
}

/** Create the code for the dictionary in the output file.
*/
void output_dictionary(FILE *of, word* dictionary, unsigned int dsize)
{
    unsigned int i,j;


    if(compress5bit_dict) {
        for(i=0; i<dsize;++i) {
            fprintf(of, "char dict%d[]={",i);
            compress_5bit(dictionary[i].w);
            for(j=0;dictionary[i].w[j];++j) {
                if(j>0)
                    fprintf(of, ", ");
                fprintf(of, "0x%x",(unsigned char)dictionary[i].w[j]);
            }
            fprintf(of, ",0x0};\n"); /* Is 0x0 absolutely necessary? */
        }
    } else if(compress_hash_dict) {
        for(i=0; i<dsize;++i) {
            compress_hash(dictionary[i].w);
        }
    }

    for(i=0; i<dsize;++i) {
        for(j=i+1; j<dsize;++j) {
            if(strcmp(dictionary[i].w, dictionary[j].w)==0 &&
                strlen(dictionary[i].w)==strlen(dictionary[j].w)) {
                printf("WARNING: repeated words in the dictionary at codes %d"
                    " and %d.\n",dictionary[i].code,dictionary[j].code);
            }
        }
    }

    fprintf(of, "const word dictionary[DSIZE]={\n");
    for(i=0; i<dsize;++i) {
        fprintf(of, TAB "{");
        if(compress5bit_dict) {
            fprintf(of, "dict%d",i);
        } else if(compress_hash_dict) {
            for(j=0;j<3;++j) {
                if(j>0)
                    fprintf(of, ", ");
                fprintf(of, "0x%x",(unsigned char)dictionary[i].w[j]);
            }
        } else {
            fprintf(of, "\"%s\"",dictionary[i].w);
        }
        fprintf(of, ",%d,%d}",dictionary[i].code,dictionary[i].t);

        if(i<dsize-1) {
            fprintf(of,",");
        }
        fprintf(of,"\n");
    }
    fprintf(of,"};\n\n");
}

/* Search for the maximum room code, to see if we can use a single byte
   to represent it. It may happen frequently, so it is worth to check that
*/
int get_max_room_code(room* world, unsigned int rsize)
{
    int maxcode=0;
    unsigned int i;

    for(i=0; i<rsize;++i) {
        if(world[i].code>maxcode)
            maxcode=world[i].code;
    }
    printf("Maxroom - obj: %d\n",maxcode);

    return maxcode;
}

/* Search for the maximum object code, to see if we can use a single byte
   to represent it. It may happen frequently, so it is worth to check that
*/
int get_max_object_code(object* obj, unsigned int osize)
{
    int maxcode=0;
    unsigned int i;

    for(i=0; i<osize;++i) {
        if(obj[i].code>maxcode)
            maxcode=obj[i].code;
    }
    return maxcode;
}


/** Create the code for the rooms' description in the output file.
    Return the total size occupied by room descriptions.
*/
unsigned int output_rooms(FILE *of, room* world, unsigned int rsize)
{
    unsigned int i,j;
    char *long_d;
    char *p;
    unsigned int size_d=0;
    unsigned int totalsize=0;

    if(compress_messages==true) {
        for(i=0; i<rsize;++i) {
            fprintf(of, "const char long_d%d[]={",world[i].code);
            totalsize+=compress(of, encodechar(world[i].long_d));
            fprintf(of, "};\n");
            fprintf(of, "const char short_d%d[]={",world[i].code);
            totalsize+=compress(of, encodechar(world[i].short_d));
            fprintf(of, "};\n");
        }
    }
    if(use_6_directions)
        fprintf(of,"#define NDIR 6\n");
    else
        fprintf(of,"#define NDIR 10\n");
    fprintf(of,"const room_code original_connections[RSIZE][NDIR]={\n");
    for(i=0; i<rsize;++i) {
        fprintf(of, TAB "{");
        for(j=0; use_6_directions==true?j<6:j<10;++j) {
            fprintf(of, "%d,", world[i].directions[j]);
        }
        fprintf(of,"}");
        if(i<rsize-1)
            fprintf(of,",");
        fprintf(of,"\n");
    }
    fprintf(of, "};\n");
    fprintf(of, "room world[RSIZE]={\n");
    for(i=0; i<rsize;++i) {
        p=encodechar(world[i].long_d);
        long_d = (char*) calloc(strlen(p)+1, sizeof(char));
        strcpy(long_d,p);

        fprintf(of, TAB "{%d,",world[i].code);
        if(compress_messages==true) {
            fprintf(of, "long_d%d",world[i].code);
        } else {
            fprintf(of, "\"%s\"", long_d);
            totalsize+=strlen(long_d);
        }
        fprintf(of, ",");
        if(compress_messages==true) {
            fprintf(of, "short_d%d",world[i].code);
        } else {
            fprintf(of, "\"%s\"", world[i].short_d);
            totalsize+=strlen(world[i].short_d);
        }
        fprintf(of, ",");
        free(long_d);
        fprintf(of, "{");
        for(j=0; use_6_directions==true?j<6:j<10;++j) {
            fprintf(of, "%d,", world[i].directions[j]);
        }
        fprintf(of, "}");
        fprintf(of, "}");
        if(i<rsize-1) {
            fprintf(of,",");
        }
        fprintf(of,"\n");
    }
    fprintf(of,"};\n\n");
    return totalsize;
}

/** Create the code for the messages in the output file.
    Return the total size in byte occupied by the messages.
*/
unsigned int output_messages(FILE *of, message* msg, unsigned int msize,
    info *header)
{
    unsigned int i,j;
    unsigned int size_d=0;
    unsigned int totalsize=0;
    char awsversion[128];
    if(compress_messages==true) {
        fprintf(of, "char areyousure[]={");
        totalsize+=compress(of, encodechar(AREYOUSURE));
        fprintf(of, "};\n");
        fprintf(of, "char exitrestart[]={");
        totalsize+=compress(of, encodechar(EXITRESTART));
        fprintf(of, "};\n");
        if(!no_header) {
            buffer[0]='\0';
            strcat(buffer,header->name);
            strcat(buffer,"\n");
            strcat(buffer,header->author);
            strcat(buffer,"\n");
            strcat(buffer,header->date);
            sprintf(awsversion, "\nAWS %s",header->version);
            strcat(buffer,awsversion);
            fprintf(of, "char header[]={");
            totalsize+=compress(of, encodechar(buffer));
            fprintf(of, "};\n");
            if(strcmp(header->description,"")!=0) {
                fprintf(of, "char headerdescription[]={");
                totalsize+=compress(of, encodechar(header->description));
                fprintf(of, "};\n");
            } else {
                no_header_description=true;
            }
        }
    }

    if(compress_messages==true||hardcoded_messages==true) {
        for(i=0; i<msize;++i) {
            if(!strip_empty_messages || strcmp(msg[i].txt,"")!=0) {
                fprintf(of, "char message%d[]=",msg[i].code);
                if(compress_messages==true) {
                    fprintf(of,"{");
                    totalsize+=compress(of, encodechar(msg[i].txt));
                    fprintf(of,"}");
                } else {
                    fprintf(of,"\"%s\"",encodechar(msg[i].txt));
                    totalsize+=strlen(buffer)+1;
                }
                fprintf(of, ";\n");
            }
        }
    }
    if(hardcoded_messages==false) {
        fprintf(of, "#define MSIZE %d\n",msize);
        fprintf(of, "message msg[MSIZE]={\n");
        for(i=0; i<msize;++i) {
            if(compress_messages==true) {
                fprintf(of, TAB "{%d,message%d}",msg[i].code,msg[i].code);
            } else {
                fprintf(of, TAB "{%d,\"%s\"}", msg[i].code,
                    encodechar(msg[i].txt));
                totalsize+=strlen(buffer)+1;
            }
            if(i<msize-1) {
                fprintf(of,",");
            }
            fprintf(of,"\n");
        }
        fprintf(of,"};\n\n");
    } else {
        if(use_6_directions) {
            fprintf(of, "char* dir[6]={\n");
            j=6;
        } else {
            fprintf(of, "char* dir[10]={\n");
            j=10;
        }
        for(i=0;i<j;++i) {
            fprintf(of, TAB "message%d",1021+i);
            if(i<j-1)
                fprintf(of, ",\n");
        }
        fprintf(of, "};\n\n");
    }
    return totalsize;
}

/** Create the code for the objects in the output file. */
void output_objects(FILE *of, object* obj, unsigned int osize)
{
    unsigned int i,j;
    if(compress_messages==true) {
        for(i=0; i<osize;++i) {
            fprintf(of, "char desc_l%d[]=",obj[i].code);
            fprintf(of,"{");
            compress(of, encodechar(obj[i].desc));
            fprintf(of,"}");
            fprintf(of, ";\n");

        }
    }

    fprintf(of,"int original_position[OSIZE]={\n" TAB);
    for(i=0; i<osize;++i)
        fprintf(of, "%d,", obj[i].position);
    fprintf(of,"};\n");

    fprintf(of, "object obj[OSIZE]={\n");
    for(i=0; i<osize;++i) {
        if(no_obj_long_desc) {
            fprintf(of, TAB "{%d,",obj[i].code);
        } else {
            fprintf(of, TAB "{%d,\"%s\",",obj[i].code,obj[i].s);
        }
        if(compress_messages==true) {
            fprintf(of, "desc_l%d,",obj[i].code);
        } else {
            fprintf(of, "\"%s\",",encodechar(obj[i].desc));
        }
        if(dont_care_size_weight==false) {
            fprintf(of, "%d,%d,%d,",obj[i].weight,obj[i].size,obj[i].position);
        } else {
            fprintf(of, "%d,",obj[i].position);
        }
        fprintf(of, "0");
        if(obj[i].attributes&ISNOTMOVABLE)
            fprintf(of, "+ISNOTMOVABLE");
        if(obj[i].attributes&ISWEREABLE)
            fprintf(of, "+ISWEREABLE");
        fprintf(of, "}");

        if(i<osize-1)
            fprintf(of,",");

        fprintf(of,"\n");
    }
    fprintf(of,"};\n\n");
}


/* Create code for HI conditions */
void output_hicond(FILE *f, char **cond, int size)
{
    int i;
    fprintf(f,"void hi_cond(void)\n{\n");
    fprintf(f, TAB "retv=true;\n");

    for(i=0; i<size; ++i) {
        process_aws(f,cond[i]);
    }
    fprintf(f, TAB "retv=false;");
    fprintf(f, TAB "return;\n");
    fprintf(f,"}\n");
}

/* Create code for LOW conditions */
void output_lowcond(FILE *f, char **cond,  int size)
{
    int i;
    fprintf(f,"void low_cond(void)\n{\n");
    fprintf(f, TAB "retv=true;\n");

    for(i=0; i<size; ++i) {
        process_aws(f,cond[i]);
    }
    fprintf(f, TAB "retv=false;\n");
    fprintf(f, TAB "return;\n");
    /*  Use a global variable for keeping track of the return value.
        The difference can be considerable in big
        adventures, check_position_marker_on there are plenty of WAIT commands. */
    fprintf(f,"}\n");
}

/* Create code for LOCAL conditions */
void output_local(FILE *f, localc* cond,  int size)
{
    unsigned int i;
    unsigned int oldroom=-1;
    boolean first=true;
    fprintf(f,"void local_cond(void)\n{\n");
    fprintf(f, TAB "retv=true;\n");

    fprintf(f, TAB "switch(current_position) {\n");
    for(i=0; i<size; ++i) {
        if(oldroom!=cond[i].room) {
            if(first==false)
                fprintf(f, TAB "break;\n");

            fprintf(f, TAB "case %d:\n", cond[i].room);
            first=false;
            oldroom=cond[i].room;
        }
        process_aws(f,cond[i].condition);
    }
    fprintf(f, TAB "}\n");

    fprintf(f, TAB "retv=false;");
    fprintf(f, TAB "return;\n");
    fprintf(f,"}\n");
}

/* Create code for the main game cycle */
void output_gameloop(FILE *f, int osize)
{
    fprintf(f,"\nvoid game_cycle(void)\n{\n");
    if(osize>255)
        fprintf(f, TAB "unsigned int k;\n");
    else
        fprintf(f, TAB "unsigned char k;\n");
    fprintf(f, TAB "boolean ve,pa;\n");
    fprintf(f, TAB "while(1){\n");
    fprintf(f, TAB TAB "current_position=next_position;\n");
    //fprintf(f, TAB TAB "++counter[125];\n");
    if(dont_use_light)
        fprintf(f, TAB TAB "if(marker[120]==false) {\n");
    else
        fprintf(f, TAB TAB
            "if(marker[120]==false&&(marker[121]||marker[122])) {\n");
    if(add_clrscr==true)
        fprintf(f, TAB TAB TAB "clear();\n");
    else
        fprintf(f,TAB TAB TAB "printnewline();\n");
    fprintf(f, TAB TAB TAB "evidence1();\n");
    fprintf(f, TAB TAB TAB "cr=&world[search_room(current_position)];\n");
    if(compress_messages==true) {
        if(hardcoded_messages==true) {
            fprintf(f,TAB TAB TAB
                "show_messagenlf(cr->short_d);\n");
        } else {
            fprintf(f,TAB TAB TAB
                "write_textsl(cr->short_d);\n");
        }
    } else {
        fprintf(f, TAB TAB TAB
            "writesameln(cr->short_d);\n");
    }
    fprintf(f,TAB TAB TAB "printspace();\n");
    fprintf(f, TAB TAB TAB "end_evidence1();\n");
    fprintf(f,TAB TAB TAB  "normaltxt();\n");
    fprintf(f,TAB TAB TAB "printspace();\n");

    if(compress_messages==true) {
        if(hardcoded_messages==true) {
            fprintf(f,TAB TAB TAB
                "show_message(cr->long_d);\n");
        } else {
            fprintf(f,TAB TAB TAB
                "write_text(cr->long_d);\n");
        }
    } else {
        fprintf(f, TAB TAB TAB
            "writeln(cr->long_d);\n");
    }
    fprintf(f, TAB TAB TAB "printnewline();\n");
    fprintf(f, TAB TAB TAB "marker[120]=true;\n");
    fprintf(f, TAB TAB TAB "ve=false;\n");
    fprintf(f, TAB TAB TAB "for(k=0;k<OSIZE;++k)\n");
    fprintf(f, TAB TAB TAB TAB "if(obj[k].position==current_position) {\n");
    fprintf(f, TAB TAB TAB TAB TAB "if(ve==false) {\n");
    if(hardcoded_messages==false)
        fprintf(f, TAB TAB TAB TAB TAB TAB "show_message(1031);\n");
    else
        fprintf(f, TAB TAB TAB TAB TAB TAB "show_message(message1031);\n");
    fprintf(f, TAB TAB TAB TAB TAB TAB "ve=true;\n");
    fprintf(f, TAB TAB TAB TAB TAB TAB "evidence2();\n");

    fprintf(f, TAB TAB TAB TAB TAB "}\n");
    if(compress_messages==true) {
        if(hardcoded_messages==true) {
            fprintf(f, TAB TAB TAB TAB TAB "show_message(obj[k].desc);\n");
        } else {
            fprintf(f, TAB TAB TAB TAB TAB "write_textsl(obj[k].desc);\n");
        }
    } else {
        fprintf(f, TAB TAB TAB TAB TAB "writeln(obj[k].desc);\n");
    }
    fprintf(f, TAB TAB TAB TAB "}\n");
    fprintf(f, TAB TAB TAB "normaltxt();\n");

    fprintf(f, TAB TAB TAB "if(marker[124]) {\n");
    fprintf(f, TAB TAB TAB TAB "pa=false;\n");
    fprintf(f, TAB TAB TAB TAB "for(k=0; k<NDIR; ++k)\n");
    fprintf(f, TAB TAB TAB TAB TAB
        "if(cr->directions[k]) {\n");
    fprintf(f, TAB TAB TAB TAB TAB TAB "if(pa==false) {\n");
    if(hardcoded_messages==false)
        fprintf(f, TAB TAB TAB TAB TAB TAB TAB "show_messagenlf(1020);\n");
    else
        fprintf(f, TAB TAB TAB TAB TAB TAB TAB
            "show_messagenlf(message1020);\n");

    fprintf(f, TAB TAB TAB TAB TAB TAB TAB"pa=true;\n");
    fprintf(f, TAB TAB TAB TAB TAB TAB "}\n");
    fprintf(f, TAB TAB TAB TAB TAB "evidence3();\n");

    if(hardcoded_messages==false)
        fprintf(f, TAB TAB TAB TAB TAB "show_messagenlf(1021+k);\n");
    else
        fprintf(f, TAB TAB TAB TAB TAB "show_messagenlf(dir[k]);\n");
    fprintf(f, TAB TAB TAB TAB TAB "printspace();\n");
    fprintf(f, TAB TAB TAB TAB "}\n");
    fprintf(f, TAB TAB TAB TAB "normaltxt();\n");
    fprintf(f, TAB TAB TAB TAB "printnewline();\n");
    fprintf(f, TAB TAB TAB "}\n");
    fprintf(f, TAB TAB "}\n");
    if(!strip_automatic_counters) {
        fprintf(f, TAB TAB "++counter[125];\n");
        fprintf(f, TAB TAB "--counter[126];\n");
        fprintf(f, TAB TAB "--counter[127];\n");
        fprintf(f, TAB TAB "--counter[128];\n");
    }
    fprintf(f, TAB TAB "hi_cond();\n");
    fprintf(f, TAB TAB "if(retv) continue;\n");
    fprintf(f, TAB TAB "printnewline();\n");

    if(hardcoded_messages==false)
        fprintf(f, TAB TAB "if(ls==0 && counter[125]<5) show_message(1012);\n");
    else
        fprintf(f, TAB TAB "if(ls==0 && counter[125]<5) "
            "show_message(message1012);\n");
    fprintf(f, TAB TAB "interrogationAndAnalysis();\n");
    fprintf(f, TAB TAB "local_cond();\n");
    fprintf(f, TAB TAB "if(retv) continue;\n");
    fprintf(f, TAB TAB "low_cond();\n");
    fprintf(f, TAB TAB "if(retv) continue;\n");
    if(hardcoded_messages==false)
        fprintf(f, TAB TAB "show_message(cv(0)?1009:1010);\n");
    else
        fprintf(f, TAB TAB "show_message(cv(0)?message1009:message1010);\n");
    fprintf(f, TAB "}\n");
    fprintf(f, "}\n");
}

void print_header(FILE *f, info *header)
{
    /* Write the code to provide the welcome message when the game starts. */
    fprintf(f, TAB "evidence2();\n");
    if(compress_messages==true) {
        if(hardcoded_messages==true) {
            fprintf(f, TAB "show_message(header);\n");
            if(!no_header_description)
                fprintf(f, TAB "show_message(headerdescription);\n");
        } else {
            fprintf(f, TAB "write_textsl(headername);\n");
            if(!no_header_description)
                fprintf(f, TAB "write_textsl(headerdescription);\n");
        }
    } else {
        if(!no_header_description) {
            fprintf(f, TAB "writeln(\"%s\\n", encodechar(header->name));
            fprintf(f, "%s\\n", encodechar(header->author));
            fprintf(f, "%s\\n", encodechar(header->date));
            fprintf(f, "%s\\n\");\n", encodechar(header->description));
            fprintf(f, TAB "writesameln(\"AWS %s\");\n",
                encodechar(header->version));

        } else {
            fprintf(f, TAB "writeln(\"%s\\n", encodechar(header->name));
            fprintf(f, "%s\\n", encodechar(header->author));
            fprintf(f, " %s\\n\");\n", encodechar(header->date));
            fprintf(f, TAB "writesameln(\"AWS %s\");\n",
                encodechar(header->version));
        }
    }
    fprintf(f, TAB "normaltxt();\n");
    fprintf(f, TAB "waitkey();\n");

}

/** Create the entry point of the game. */
void create_main(FILE *f,info *header)
{
    fprintf(f, "\nint main(void)\n{\n");
    fprintf(f, TAB "restart();\n");
    fprintf(f, TAB "init_term();\n");

    if(!no_header)
        print_header(f, header);

    fprintf(f, TAB "game_cycle();\n");
    fprintf(f, TAB "return 0;\n");
    fprintf(f, "}\n");
}

/* Print a help. */
void print_help(char *name)
{
    printf("Adventure Writing System to C compiler, version %s\n", VERSION);
    printf("Davide Bucci\n\n");
    printf("Usage: %s [options] inputfile.aws outputfile\n\n",name);
    printf( "then compile (along with file inout.c) using your favourite "
        "C compiler.\n\n");

    printf("Available options:\n"
           " -h  this help\n"
           " -u  convert UTF-8 characters into standard ASCII chars.\n"
           "        è -> e   é -> e\n"
           " -r  same as -u, but keep accents as separate chars.\n"
           "        è -> e'  è -> e`\n"
           " -s  same as -u, but only employs the single accent '.\n"
           "        é -> e'  è -> e'\n"
           " -c  compress text with Huffman algorithm.\n"
           " -d  employ 6 directions instead of 10.\n"
           " -m  employ hardcoded messages instead of an array.\n"
           " -n  do not clear screen every time a new room is shown.\n"
           " -v  or --version print version and exit\n"
           " -w  don't check for size and weight of objects (counter 119, 120\n"
           "     and 124 are not used).\n"
           " -k  don't output header.\n"
           " -kk strip empty messages and automatic counters.\n"
           " -5  use 5-bit compression for the dictionary.\n"
           " -3  use 3-byte hash code for dictionary.\n"
           " -l  don't take into account light/dark situations.\n"
           " -f  <filename> specify the name of the file to be used for the\n"
           "     configuration. Default is config.h.\n"
           "     NOTE: if this option is used, you should compile the files\n"
           "     with the macro CONFIG_FILENAME defined to the configuration\n"
           "     file name, within \". For example, for gcc\n"
           "     gcc -DCONFIG_FILENAME=\\\"test.h\\\" ...\n"
           " --verbose write plenty of things.\n");

    printf("\n");
}

/*  Process options from the command line.
    Return 1 if the option has been recognized and treated, zero otherwise.
*/
unsigned int process_options(char *arg, char *name)
{
    static boolean require_config_file;

    /* This condition is true if the previous execution of process_option
       has required a name for the configuration file.
    */
    if(require_config_file) {
        config_file=calloc(strlen(arg)+1, sizeof(char));
        strcpy(config_file, arg);
        require_config_file=false;
        return 1;
    }

    /* Process normal options. */
    if(strcmp(arg, "-h")==0) {
        print_help(name);
        return 1;
    } else if (strcmp(arg, "-u")==0) {
        convert_utf8=true;
        convert_accents=false;
        return 1;
    } else if (strcmp(arg, "-r")==0) {
        convert_utf8=true;
        convert_accents=true;
        return 1;
    } else if (strcmp(arg, "-s")==0) {
        convert_utf8=true;
        convert_accents=true;
        convert_accent_alt=true;
        return 1;
    } else if (strcmp(arg, "-d")==0) {
        use_6_directions=true;
        return 1;
    } else if (strcmp(arg, "-c")==0) {
        compress_messages=true;
        compress_descriptions=true;
        return 1;
    } else if (strcmp(arg, "-m")==0) {
        hardcoded_messages=true;
        return 1;
    } else if (strcmp(arg, "-n")==0) {
        add_clrscr=false;
        return 1;
    } else if (strcmp(arg, "-w")==0) {
        dont_care_size_weight=true;
        return 1;
    } else if (strcmp(arg, "-k")==0) {
        no_header=true;
        //strip_empty_messages=true;    // It doesn't work if a message is used
        strip_automatic_counters=true;
        return 1;
    } else if (strcmp(arg, "-kk")==0) {
        //strip_empty_messages=true;
        strip_automatic_counters=true;
        return 1;
    } else if (strcmp(arg, "-v")==0 || strcmp(arg, "--version")==0) {
        printf("This is AWS2C version %s\n", VERSION);
        exit(0);
    } else if (strcmp(arg, "-5")==0) {
        compress5bit_dict=true;
        if(compress_hash_dict) {
            fprintf(stderr, "ERROR: options -3 and -5 are not compatible.\n");
            exit(1);
        }
        return 1;
    } else if (strcmp(arg, "-3")==0) {
        if(compress5bit_dict) {
            fprintf(stderr, "ERROR: options -3 and -5 are not compatible.\n");
            exit(1);
        }
        compress_hash_dict=true;
        return 1;
    } else if (strcmp(arg, "-l")==0) {
        dont_use_light=true;
        return 1;
    } else if (strcmp(arg, "-f")==0) {
        require_config_file=true;
        return 1;
    } else if (strcmp(arg, "--verbose")==0) {
        verbose=true;
        return 1;
    }
    return 0;
}

/* Check if one string contains a second one */
int contains(char *s1, char *s2)
{
    int i=0, j=0, t=0;
    while (s1[i]!=0) {
        t=i;
        for(j=0; s2[j]!='\0'; ++j)
            if(tolower(s1[i++])!=tolower(s2[j]))
                break;
        if(s2[j]=='\0')
            return t;
    };
    return -1;
}


/* Check the code and set up some configuration variables to improve the
   code generation efficiency.
*/
void code_analysis(char **commands, unsigned int size)
{
    unsigned int i, s, e;
    /* Check if JUMP is used more than once */
    for(i=0; i<size; ++i) {
        e=contains(commands[i],"endif");

        s=contains(commands[i],"goto");
        if(s>=0 && s<e) {
            ++number_of_jumps;
        }
        s=contains(commands[i],"drop");
        if(s>=0 && s<e) {
            ++number_of_drops;
        }

    }
}

/* Check the code and set up some configuration variables to improve the
   code generation efficiency.
*/
void code_analysisLC(localc *commands, unsigned int size)
{
    unsigned int i,s,e;
    /* Check if JUMP is used more than once */
    for(i=0; i<size; ++i) {
        e=contains(commands[i].condition,"endif");

        s=contains(commands[i].condition,"goto");
        if(s>=0 && s<e) {
            ++number_of_jumps;
        }
        s=contains(commands[i].condition,"drop");
        if(s>=0 && s<e) {
            ++number_of_drops;
        }
    }
}

void set_up_optimization(void)
{
    printf("Number of GOTO commands: %d\n", number_of_jumps);
    if(number_of_jumps>1)
        jump_as_function=true;
    else
        jump_as_function=false;

    printf("Number of DROP commands: %d\n", number_of_drops);
    if(number_of_drops>1)
        drop_as_function=true;
    else
        drop_as_function=false;

}

/* Entry point of the program. */
int main(int argc, char **argv)
{
    FILE *f;
    FILE *of;
    unsigned int dsize;
    unsigned int rsize_bytes;
    unsigned int osize;
    unsigned int msize;
    unsigned int msize_bytes;
    unsigned int i;
    int max_room_code;
    int max_obj_code;
    info *header;
    word* dictionary;
    object* objects;
    message* msg;
    char **hicond;
    int hicondsize;
    char **lowcond;
    int lowcondsize;
    localc* localcond;
    int localcondsize;
    unsigned int argumentr=1;

    init_analysis();
    while (argumentr<argc && process_options(argv[argumentr], argv[0])) {
        ++argumentr;
    }
    if (argumentr+1!=argc-1) {
        fprintf(stderr,"Not enough arguments on the command line.\n");
        return 1;
    }
    printf("Reading %s\n",argv[argumentr]);
    f=fopen(argv[argumentr],"r");

    if(f==NULL) {
        printf("Could not open input file.\n");
        return 1;
    }
    printf("Read header: ");
    fflush(stdout);
    if ((header=read_header(f))==NULL)
        exit(1);

    printf("done\n");

    printf("Read number of CONDIZIONIHI: ");
    hicondsize=get_hi_cond_size(f);
    printf("%d\n",hicondsize);
    printf("Read CONDIZIONIHI: ");
    if((hicond=read_cond(f, hicondsize))==NULL && hicondsize>0)
        exit(1);

    printf("done\n");

    printf("Read number of CONDIZIONILOW: ");
    lowcondsize=get_low_cond_size(f);
    printf("%d\n",lowcondsize);
    printf("Read CONDIZIONILOW: ");
    if((lowcond=read_cond(f, lowcondsize))==NULL && lowcondsize>0)
        exit(1);
    printf("done\n");

    printf("Read number of CONDIZIONILOCALI: ");
    localcondsize=get_local_cond_size(f);
    printf("%d\n",localcondsize);
    printf("Read CONDIZIONILOCALI: ");
    if((localcond=read_local(f, localcondsize))==NULL && localcondsize>0)
        exit(1);
    printf("done\n");

    printf("Get dictionary size: ");
    dsize=get_dict_size(f);
    printf("%d\n",dsize);
    printf("Read dictionary: ");
    if((dictionary=read_dictionary(f,dsize))==NULL && dsize>0)
        exit(1);
    printf("done\n");

    printf("Get the number of rooms in use: ");
    rsize=get_room_number(f);
    printf("%d\n",rsize);
    printf("Read world: ");
    if((world=read_rooms(f,rsize))==NULL)
        exit(1);
    printf("done\n");

    printf("Get the number of messages in use: ");
    msize=get_messages_number(f);
    printf("%d\n",msize);
    printf("Read messages: ");
    if((msg=read_messages(f,msize))==NULL)
        exit(1);
    printf("done\n");

    printf("Get the number of objects recognised by the game: ");
    osize=get_objects_number(f);
    printf("%d\n",osize);
    printf("Read objects: ");
    if((objects=read_objects(f, osize))==NULL) {
        printf("No readable objects in this game!\n");
        objects=(object*)calloc(1, sizeof(object));
        osize=0;
    }
    fclose(f);
    printf("done\n");
    printf("Analyze AWS code: ");
    code_analysisLC(localcond, localcondsize);
    code_analysis(hicond, hicondsize);
    code_analysis(lowcond, lowcondsize);
    printf("done\n");

    set_up_optimization();

    printf("Create the output file\n");
    of=fopen(argv[argumentr+1],"w");
    if(of==NULL) {
        return 1;
    }
    no_of_errors=0;
    max_room_code=get_max_room_code(world, rsize);
    max_obj_code =get_max_object_code(objects, osize);
    output_header(of, max_room_code,max_obj_code, dsize, rsize, osize,
        header->name);
    if(compress_messages==true || compress_descriptions==true) {
        output_decoder(of);
    }
    output_dictionary(of, dictionary, dsize);
    rsize_bytes=output_rooms(of, world, rsize);
    msize_bytes=output_messages(of, msg, msize, header);
    output_objects(of, objects, osize);
    output_utility_func(of,header,rsize, osize,max_room_code);

    output_hicond(of, hicond, hicondsize);

    output_lowcond(of, lowcond, lowcondsize);
    output_local(of,localcond, localcondsize);
    output_gameloop(of, osize);
    create_main(of,header);
    output_optional_func(of,max_room_code);
    fclose(of);
    printf("File %s created\n",argv[argumentr+1]);
    printf("Size occupied by room descriptions: %d bytes\n", rsize_bytes);
    printf("Size occupied by messages: %d bytes\n", msize_bytes);
    printf("No of critical errors: %d\n",no_of_errors);
    if(no_of_errors>0)
        return 1;

    return 0;
}