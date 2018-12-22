/*

    AWS to C converter 1.0 by Davide Bucci

AWS stands for Adventure Writing System and is a program developed by
Aristide Torrelli to write interactive fiction games:

http://www.aristidetorrelli.it/aws3/AWS.html

The game developed with AWS is described by a compact file that contains the
vocabulary, the messages, the descriptions and all the logic needed for the
game.

Its structure of AWS is relatively simple yet powerful and I decided to write
this little converter that automagically generates C code that implements the
logic described by the game.

The parser and some input/output functions are contained in an external file
called inout.c that can be customised to the target machine.

I tested this system with gcc and the generated files can be compiled with
any reasonably standard C compiler. I tested them with cc65 to target
old Commodore machines.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "aws.h"
#include "compress.h"

#define VERSION "1.2, september-december 2018"

/* TO DO

- Finish implementing the remaining actions, functions and decisions.
- Free the allocated memory before quitting the program.
- Test, test, test!

*/

#define BUFFERSIZE 16000
char buffer[BUFFERSIZE];
char function_res[BUFFERSIZE];
#define TAB "    "


#define start_function() function_res[0]='\0';

typedef struct localc_t {
    unsigned int room;
    char *condition;
} localc;

unsigned int no_of_errors;
boolean convert_utf8=false;
boolean convert_accents=false;
boolean convert_accent_alt=false;
boolean compress_messages=false;
boolean compress_descriptions=false;
boolean use_6_directions=false;
boolean shortcuts=true;
boolean hardcoded_messages=false;
boolean add_clrscr=true;

boolean complete_shortcut=false;

boolean need_searchw=false;
boolean need_vov=false;
boolean need_vovn=false;
boolean need_non1=false;
boolean need_cvn=false;
boolean need_hold=false;
boolean need_cvna=false;
boolean need_sendallroom=false;

typedef struct conv_t {
    char *orig;
    char *conv;
    char accent;
    char accent_alt;
} conv;

/* This is a minimal conversion that should work for the Italian language
   (at least). It is used to the conversion between UTF-8 chars and standard
   ASCII characters, plus the accents. */

#define CONVSIZE 25

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
    as ".
    Exploits buffer.
*/
char *encodechar(char *input)
{
    unsigned int i,j,k,t;
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
word *read_dictionary(FILE *f, unsigned int size)
{
    unsigned int cw;
    fpos_t pos;
    char *errorp="Could not allocate enough memory for the dictionary.\n";
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
            dictionary[cw].t=ADVERB;
        } else if(strcmp(buffer, "SEPARATORE")==0) {
            dictionary[cw].t=SEPARATOR;
        } else if(strcmp(buffer, "NOME")==0) {
            dictionary[cw].t=NAME;
        } else if(strcmp(buffer, "ATTORE")==0) {
            dictionary[cw].t=ACTOR;
        } else if(strcmp(buffer, "AGGETTIVO")==0) {
            dictionary[cw].t=ADJECTIVE;
        } else {
            printf("Unknown word type: %s.\n",buffer);
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
    if(buffer[sl - 2]=='\r')    // This is needed for Windows
        buffer[sl - 2] = '\0';  // style newline code (\r\n).
    else
        buffer[sl - 1] = '\0';
    return buffer;
}

/** Get the number of the rooms in the game.
*/
unsigned int get_room_number(FILE *f)
{
    fpos_t pos;
    unsigned int counter=0;
    unsigned int sl=0;
    while(fscanf(f,"%80s",buffer)==1){
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
char **read_cond(FILE*f, unsigned int size)
{
    unsigned int i;
    char **cond;
    char *errorp="Could not allocate enough memory for the conditions.\n";

    cond = (char**)calloc(size, sizeof(char*));
    if (cond==NULL) {
        printf("%s",errorp);
        return NULL;
    }
    getlinep(f);
    for(i=0; i<size;++i) {
        getlinep(f);
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
localc* read_local(FILE*f, unsigned int size)
{
    unsigned int i,r;
    localc* cond;
    char *errorp="Could not allocate enough memory for the local conditions.\n";

    cond = (localc*)calloc(size, sizeof(localc));
    if (cond==NULL) {
        printf("%s",errorp);
        return NULL;
    }
    getlinep(f);
    for(i=0; i<size;++i) {
        getlinep(f);
        if(sscanf(buffer,"%d",&r)!=1) {
            printf("\nCould not read the room number!\n");
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
    }
    return cond;
}

/** Get the rooms in the game
*/
room* read_rooms(FILE *f, unsigned int size)
{
    char *errorp="Could not allocate enough memory for the room description.\n";
    unsigned int i,j;
    room *world = (room*)calloc(size, sizeof(room));
    if (world==NULL) {
        printf("%s",errorp);
        return NULL;
    }

    for(i=0; i<size;++i) {
        getlinep(f);
        getlinep(f);
        sscanf(buffer, "%d",&(world[i].code));
        getlinep(f);
        world[i].long_d=calloc(strlen(buffer)+1,sizeof(char));
        strcpy(world[i].long_d,buffer);
        if(compress_descriptions==true)
            analyze(encodechar(world[i].long_d));
        getlinep(f);
        world[i].s=calloc(strlen(buffer)+1,sizeof(char));
        strcpy(world[i].s,buffer);
        if(compress_descriptions==true)
            analyze(encodechar(world[i].s));
        getlinep(f);
        world[i].short_d=calloc(strlen(buffer)+1,sizeof(char));
        strcpy(world[i].short_d,buffer);
        if(compress_descriptions==true)
            analyze(encodechar(world[i].short_d));
        for(j=0;j<10;++j) {
            if(fscanf(f,"%d",&(world[i].directions[j]))!=1) {
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
object* read_objects(FILE *f, unsigned int size)
{
    char *errorp="Could not allocate enough memory for the objects.\n";
    unsigned int i,j;
    object *obj = (object*)calloc(size, sizeof(object));
    if (obj==NULL) {
        printf("%s",errorp);
        return NULL;
    }

    getlinep(f);

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
            obj[i].isnotmovable=false;
        } else {
            obj[i].isnotmovable=true;
        }

        getlinep(f);
        if(strcmp(buffer,"FALSE")) {
            obj[i].isnotwereable=false;
        } else {
            obj[i].isnotwereable=true;
        }
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
    return in;
}

/** Get the messages in the game
*/
message* read_messages(FILE *f, unsigned int size)
{
    char *errorp="Could not allocate enough memory for messages.\n";
    unsigned int i,j;
    message *msg = (message*)calloc(size, sizeof(message));
    if (msg==NULL) {
        printf("%s",errorp);
        return NULL;
    }

    getlinep(f);
    for(i=0; i<size;++i) {
        getlinep(f);
        sscanf(buffer, "%d",&(msg[i].code));

        getlinep(f);
        msg[i].txt=calloc(strlen(buffer)+1,sizeof(char));
        strcpy(msg[i].txt,buffer);
        if(compress_messages==true)
            analyze(encodechar(msg[i].txt));
    }
    return msg;
}

/** Get the number of the objects in the game.
*/
unsigned int get_objects_number(FILE *f)
{
    fpos_t pos;
    unsigned int counter=0;
    unsigned int sl=0;
    while(fscanf(f,"%80s",buffer)==1){
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
unsigned int get_messages_number(FILE *f)
{
    fpos_t pos;
    unsigned int counter=0;
    unsigned int sl=0;
    while(fscanf(f,"%80s",buffer)==1){
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
unsigned int get_hi_cond_size(FILE *f)
{
    fpos_t pos;
    unsigned int counter=0;
    unsigned int sl=0;
    while(fscanf(f,"%80s",buffer)==1){
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
    return counter-1;
}

/** Get the number of "low conditions" in the file.
*/
unsigned int get_low_cond_size(FILE *f)
{
    fpos_t pos;
    unsigned int counter=0;
    unsigned int sl=0;
    while(fscanf(f,"%80s",buffer)==1){
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
    return counter-1;
}
/** Get the number of "local conditions" in the file.
*/
unsigned int get_local_cond_size(FILE *f)
{
    fpos_t pos;
    unsigned int counter=0;
    unsigned int sl=0;
    while(fscanf(f,"%80s",buffer)==1){
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
    return (counter-1)/2;
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
        strcon(function_res,"obj[search_object(");
        scanpos=process_functions(line,scanpos);
        strcon(function_res,")].weight");
    } else if(strcmp(token,"ACTORNO")==0) {
        strcon(function_res,"actor");
    } else {
        if(sscanf(token, "%d",&value)==1) {
            sprintf(token, "%d", value);
            strcon(function_res,token);
        } else {
            printf("Function not recognized %s\n", token);
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
                        if(hardcoded_messages==false)
                            fprintf(f, "1) amsm(%s,%s,%d,%s);",
                                arg1,arg2,polarity,function_res);
                        else
                            fprintf(f, "1) amsm(%s,%s,%d,message%s);",
                                arg1,arg2,polarity,function_res);
                    }
                }
            }
        }
    }
    if(proc==false)
        fprintf(f, "current_position==%s", arg1);
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
    fprintf(f, "marker[%s]==true", function_res);
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
/** VERB */
unsigned int decision_verb(FILE *f, char *line, unsigned int scanpos)
{
    unsigned int sp,at,tt;
    boolean proc=false;
    char *arg1,*arg2;
    start_function();
    tt=scanpos=process_functions(line, scanpos);
    sp=peek_token(line, scanpos);
    if(shortcuts==true&&(strcmp(next,"AND")==0)) {
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
                    fprintf(f,"cvna(%s,%s,%s)", arg1, arg2, function_res);
                    need_cvna=true;
                    proc=true;
                }
            } else if(strcmp(next,"OR")==0) {
                fprintf(f, "verb==%s", arg1);
                scanpos=tt;
                proc=true;
            }
            if(proc==false) {
                fprintf(f, "cvn(%s,%s)", arg1,arg2);
                need_cvn=true;
            }
            free(arg1);
            free(arg2);
        } else {
            fprintf(f, "verb==%s", function_res);
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
                        fprintf(f,"vovn(%s,%s,%s)", arg1, arg2,function_res);
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
        fprintf(f, "verb==%s", function_res);
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
    fprintf(f, "(get_object_position(%s)!=current_position)&&",
        function_res);
    fprintf(f, "(object_is_carried(%s)==false)", function_res);

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
    return scanpos;
}
/** NO1LT */
unsigned int decision_no1lt(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "(noun1>0&&noun1<%s)", function_res);
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
    fprintf(f, "obj[search_object(%s)].isnotmovable==false",
        function_res);
    return scanpos;
}
/** ISNOTMOVABLE */
unsigned int decision_isnotmovable(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "obj[search_object(%s)].isnotmovable==true",
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
    fprintf(f, TAB TAB "goto return1;\n");
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
    if(strcmp("1036",function_res)==0) {
        // Message 1036 is particular: it points to the name1 introduced by
        // the player
        fprintf(f, TAB TAB "printf(\"%%s\\n\",searchw(noun1));\n");
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
unsigned int action_exit(FILE *f, char *line, unsigned int scanpos)
{

    fprintf(f, TAB TAB "leave(); exit(0);\n");
    return scanpos;
}
/** EXIT */
unsigned int action_quit(FILE *f, char *line, unsigned int scanpos)
{
    fprintf(f, TAB TAB "leave(); exit(0);\n");
    return scanpos;
}
/** INVE */
unsigned int action_inve(FILE *f, char *line, unsigned int scanpos)
{
    fprintf(f, TAB TAB "inventory();\n");
    return scanpos;
}
/** move */
unsigned int action_move(FILE *f, char *line, unsigned int scanpos, unsigned int dir)
{
    unsigned int position, value;
    /*fprintf(f, TAB TAB "if(move(%d)==1) return 1;\n", dir-1);*/
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
    fprintf(f, TAB TAB "drop(%s);\n",function_res);
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

    fprintf(f, TAB TAB "set_object_position(%s,%s);\n",
        arg1, function_res);
    free(arg1);
    return scanpos;
}
/** OKAY */
unsigned int action_okay(FILE *f, char *line, unsigned int scanpos)
{
    if(hardcoded_messages==false)
        fprintf(f, TAB TAB "show_message(1000);\n");
    else
        fprintf(f, TAB TAB "show_message(message1000);\n");
    //fprintf(f, TAB TAB "return 1;\n");
    fprintf(f, TAB TAB "goto return1;\n");
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
    fprintf(f, TAB TAB "if(get(%s)!=0) goto return1;\n", function_res);

    return scanpos;
}
/** GETALL */
unsigned int action_getall(FILE *f, char *line, unsigned int scanpos)
{
    fprintf(f, TAB TAB "for(dummy=0; dummy<OSIZE;++dummy)\n");
    fprintf(f, TAB TAB TAB "odummy=&obj[dummy];\n");
    fprintf(f, TAB TAB TAB "if(odummy->position==current_position"
        "&&counter[124]+odummy->size>counter[121]"
        "&&counter[120]+odummy->weight>counter[122]) {\n");
    fprintf(f, TAB TAB TAB TAB "odummy->position=CARRIED;\n");
    fprintf(f, TAB TAB TAB TAB "++counter[119];\n");
    fprintf(f, TAB TAB TAB TAB "counter[120]+=odummy->weight;\n");
    fprintf(f, TAB TAB TAB TAB "counter[124]+=odummy->size;\n");
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
    fprintf(f, TAB TAB TAB TAB "--counter[119];\n");
    fprintf(f, TAB TAB TAB TAB
        "counter[120]-=odummy->weight;\n");
    fprintf(f, TAB TAB TAB TAB
        "counter[124]-=odummy->size;\n");
    fprintf(f, TAB TAB TAB "}\n");
    fprintf(f, TAB TAB "}\n");
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
    fprintf(f, TAB TAB "world[search_room(%s)].directions[(%s)-1]=%s;\n",
        arg1, arg2, function_res);
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
        "obj[search_object(%s)].position=obj[search_object(%s)].position;\n",
        arg1,function_res);
    fprintf(f, TAB TAB "obj[search_object(%s)].position=dummy;\n",
        function_res);
    free(arg1);
    return scanpos;
}
/** WAIT */
unsigned int action_wait(FILE *f, char *line, unsigned int scanpos)
{
    fprintf(f, TAB TAB "goto return1;\n");
    return scanpos;
}
/** LF */
unsigned int action_lf(FILE *f, char *line, unsigned int scanpos)
{
    fprintf(f, TAB TAB "writeln(\"\");\n");
    return scanpos;
}
/** OBJ */
unsigned int action_obj(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    if(compress_messages==true) {
        if(hardcoded_messages==false) {
            fprintf(f,TAB TAB "write_text(obj[search_object(%s)].desc);\n",
                function_res);
        } else {
            fprintf(f,TAB TAB "show_message(obj[search_object(%s)].desc);\n",
                function_res);
        }
    } else {
        fprintf(f, TAB TAB "writeln(obj[search_object(%s)].desc);",
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
        "if(odummy->isnotwereable==false&&(odummy->position==CARRIED||"
        "odummy->position==current_position)){\n");
    fprintf(f, TAB TAB TAB"odummy->position=WEARED;\n");
    fprintf(f, TAB TAB TAB "++counter[118];\n");
    fprintf(f, TAB TAB "} else if(odummy->position==WEARED) {\n");
    if(hardcoded_messages==false)
        fprintf(f, TAB TAB TAB "show_message(1019);\n");
    else
        fprintf(f, TAB TAB TAB "show_message(message1019);\n");
    fprintf(f, TAB TAB TAB "goto return1;\n");
    fprintf(f, TAB TAB "} else {\n");
    if(hardcoded_messages==false)
        fprintf(f, TAB TAB TAB "show_message(1010);\n");
    else
        fprintf(f, TAB TAB TAB "show_message(message1010);\n");
    fprintf(f, TAB TAB TAB "goto return1;\n");
    fprintf(f, TAB TAB "}\n");
    return scanpos;
}
/** UNWEAR */
unsigned int action_unwear(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, TAB TAB "dummy=search_object(%s);\n",function_res);
    fprintf(f, TAB TAB "odummy=&obj[dummy];\n");

    fprintf(f, TAB TAB "if(odummy->position==WEARED){\n");
    fprintf(f, TAB TAB TAB "odummy->position=CARRIED;\n");
    fprintf(f, TAB TAB TAB "--counter[118];\n");
    fprintf(f, TAB TAB "} else {\n");
    if(hardcoded_messages==false)
        fprintf(f, TAB TAB TAB "show_message(1010);\n");
    else
        fprintf(f, TAB TAB TAB "show_message(message1010);\n");
    fprintf(f, TAB TAB TAB "goto return1;\n");
    fprintf(f, TAB TAB "}\n");
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
/** DIMENS */
unsigned int action_dimens(FILE *f, char *line, unsigned int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, TAB TAB "counter[121]=%s;\n",function_res);
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

    if(strcmp(token, "IF")!=0) {
        printf("Unrecognised start of aws condition %s instead of IF.\n",
            token);
        ++no_of_errors;
        return;
    }
    fprintf(f,TAB "// %s\n",line);
    fprintf(f,TAB "if(");
    while(1) {
        scanpos=get_token(line, scanpos);
        if(strcmp(token,"AT")==0) {
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
        } else if(strcmp(token,"CTRGT")==0) {
            scanpos=decision_ctrgt(f, line, scanpos);
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
        } else if(strcmp(token,"ISWEARING")==0) {
            scanpos=decision_iswearing(f, line, scanpos);
        } else if(strcmp(token,"ISNOTWEARING")==0) {
            scanpos=decision_isnotwearing(f, line, scanpos);
        } else if(strcmp(token,"OBJLOCEQ")==0) {
            scanpos=decision_objloceq(f, line, scanpos);
        } else if(strcmp(token,"OBJLOCGT")==0) {
            scanpos=decision_objlocgt(f, line, scanpos);
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
        } else if(strcmp(token,"OR")==0) {
            //shortcuts=false;
            fprintf(f,"||");
        } else if(strcmp(token,"AND")==0) {
            shortcuts=true;
            // this is a trick to have the behaviour of AND as it is in AWS.
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
            // ??
            printf("RESTART is not implemented\n");
        } else if(strcmp(token,"LOAD")==0) {
            // ??
            printf("LOAD is not implemented\n");
        } else if(strcmp(token,"SAVE")==0) {
            // ??
            printf("SAVE is not implemented\n");
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
void output_header(FILE *of)
{
    fprintf(of,"/* Adventure Writing System, file generated by aws2c */\n\n");
    fprintf(of,"#include<stdio.h>\n");
    fprintf(of,"#include<stdlib.h>\n");
    if(use_6_directions==true) {
        fprintf(of,"#define DIR_REDUCED\n");
    }
    fprintf(of,"#include\"aws.h\"\n\n");
    fprintf(of,"#include\"inout.h\"\n");
    fprintf(of,"#include\"systemd.h\"\n\n");
    fprintf(of,"extern unsigned int verb;\nextern unsigned int noun1;\nextern unsigned int noun2;\n"
        "extern unsigned int adve;\nextern unsigned int actor;\nextern unsigned int adjective;\n");
    fprintf(of, "unsigned int dummy;\n");
    fprintf(of, "#define CARRIED 1500\n");
    fprintf(of, "#define WEARED 1600\n");
    fprintf(of,"\n");

}

void output_optional_func(FILE *of)
{
    if(need_searchw) {
        fprintf(of,"char *nonestr=\"\";\n");
        fprintf(of,"char *searchw(unsigned int w)\n{\n");
        fprintf(of, TAB "int i;\n");
        fprintf(of, TAB "for(i=0;i<DSIZE;++i)\n");
        fprintf(of, TAB TAB "if(w==dictionary[i].code)\n");
        fprintf(of, TAB TAB TAB "return dictionary[i].w;\n");
        fprintf(of, TAB "return nonestr;\n");
        fprintf(of, "}\n");
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
        fprintf(of, "    return (verb==v1||verb==v2)&&noun1==n;\n");
        fprintf(of, "}\n");
    }
    if(need_non1) {
    /* Check among two nouns1 */
        fprintf(of, "boolean non1(unsigned int n1, unsigned int n2)\n");
        fprintf(of, "{\n");
        fprintf(of, "    return noun1==n1||noun1==n2;\n");
        fprintf(of, "}\n");
    }
    if(need_cvn) {
        /* Check for a name and noun */
        fprintf(of, "boolean cvn(unsigned int v, unsigned int n)\n");
        fprintf(of, "{\n");
        fprintf(of, "    return verb==v&&noun1==n;\n");
        fprintf(of, "}\n");
    }
    if(need_cvna) {
        /* If a name and a noun and avai conditions are given */
        fprintf(of,
            "unsigned int cvna(unsigned int v, unsigned int n, "
            "unsigned int o)\n");
        fprintf(of, "{\n    dummy=get_object_position(o);\n");
        fprintf(of, "    return verb==v&&noun1==n&&"
            "(dummy==current_position||dummy==CARRIED);\n");
        fprintf(of, "}\n");
    }
    if(need_sendallroom) {
        fprintf(of, "void sendallroom(unsigned int s)\n{\n");
        fprintf(of, TAB "for(dummy=0; dummy<OSIZE;++dummy){\n");
        fprintf(of, TAB TAB "odummy=&obj[dummy];\n");
        fprintf(of, TAB TAB
            "if(odummy->position==CARRIED) {\n");
        fprintf(of, TAB TAB TAB
            "odummy->position=s;\n");
        fprintf(of, TAB TAB TAB "--counter[119];\n");
        fprintf(of, TAB TAB TAB
            "counter[120]-=odummy->weight;\n");
        fprintf(of, TAB TAB TAB
            "counter[124]-=odummy->size;\n");
        fprintf(of, TAB TAB "}\n");
        fprintf(of, TAB "}\n");
        fprintf(of, "}\n");
    }
    if(need_hold) {
        fprintf(of, "void hold(unsigned int p)\n{\n");
        fprintf(of, TAB "for(dummy=0; dummy<p; ++dummy) {wait1s();}\n");
        fprintf(of, "}\n");
    }
}

void output_utility_func(FILE *of)
{
    fprintf(of,"unsigned int current_position;\n");
    fprintf(of,"unsigned int next_position;\n");
    fprintf(of,"extern unsigned int ls;\n");
    fprintf(of,"boolean marker[129];\n");
    fprintf(of,"int counter[129];\n");
    fprintf(of,"object *odummy;\n\n");
    /* Introduces the prototype here, function will be included only if 
       necessary after having analyzed the file. */
    fprintf(of,"char *searchw(unsigned int w);\n");

    fprintf(of,"unsigned int search_object(unsigned int o)\n{\n");
    fprintf(of,TAB "unsigned int i;\n");
    fprintf(of,TAB "for(i=0; i<OSIZE;++i)\n");
    fprintf(of,TAB TAB "if(obj[i].code==o)\n");
    fprintf(of,TAB TAB TAB "return i;\n");
    fprintf(of,TAB "return 0;\n}\n\n");

    fprintf(of,"unsigned int search_room(unsigned int r)\n{\n");
    fprintf(of,TAB "unsigned int i;\n");
    fprintf(of,TAB "for(i=0; i<RSIZE;++i)\n");
    fprintf(of,TAB TAB "if(world[i].code==r)\n");
    fprintf(of,TAB TAB TAB "return i;\n");
    fprintf(of,TAB "return 0;\n}\n\n");

    if(hardcoded_messages==false) {
        if(compress_messages==true) {
            fprintf(of,"void write_textsl(char *m)\n{\n");
            fprintf(of,TAB "char r;\n");
            fprintf(of,TAB "decode_start(m);\n");
            fprintf(of,TAB "do {\n");
            fprintf(of,TAB TAB "r=decode();\n");
            fprintf(of,TAB TAB "writesameln(decompress_b);\n");
            fprintf(of,TAB "} while(r!=0);\n");
            fprintf(of, "}\n");
            fprintf(of,"void write_text(char *m)\n{\n");
            fprintf(of,TAB "write_textsl(m);\n");
            fprintf(of,TAB "writeln(\"\");\n");
            fprintf(of, "}\n");
        }
        fprintf(of,"void show_messagenlf(unsigned int m)\n{\n");
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

        fprintf(of,"void show_message(unsigned int m)\n{\n");
        fprintf(of,TAB "show_messagenlf(m);\n");
        fprintf(of,TAB "writeln(\"\");\n");
        fprintf(of, "}\n\n");
    } else {

        fprintf(of,"void show_messagenlf(char *m)\n{\n");
        if(compress_messages==true) {
            fprintf(of,TAB "char r;\n");
            fprintf(of,TAB "decode_start(m);\n");
            fprintf(of,TAB "do {\n");
            fprintf(of,TAB TAB "r=decode();\n");
            fprintf(of,TAB TAB "writesameln(decompress_b);\n");
            fprintf(of,TAB "} while(r!=0);\n");
        } else {
            fprintf(of,TAB "writesameln(m);\n");
        }
        fprintf(of, "}\n\n");

        fprintf(of,"void show_message(char *m)\n{\n");
        fprintf(of,TAB "show_messagenlf(m);\n");
        fprintf(of,TAB "writeln(\"\");\n");
        fprintf(of, "}\n\n");
    }
    fprintf(of,"void inventory(void)\n{\n");
    fprintf(of,TAB "unsigned int i, gs=0;\n");
    if(hardcoded_messages==false)
        fprintf(of,TAB "show_message(1032);\n");
    else
        fprintf(of,TAB "show_message(message1032);\n");
    fprintf(of,TAB "for(i = 0; i<OSIZE; ++i) {\n");
    fprintf(of,TAB TAB 
        "if(obj[i].position==CARRIED||obj[i].position==WEARED) {\n");
    fprintf(of,TAB TAB TAB "++gs;\n");
    fprintf(of,TAB TAB TAB "evidence2();\n");
    if(compress_messages==true) {
        if(hardcoded_messages==false) {
            fprintf(of,TAB TAB TAB "write_textsl(obj[i].desc);\n");
        } else {
            fprintf(of,TAB TAB TAB "show_messagenlf(obj[i].desc);\n");
        }
    } else {
        fprintf(of,TAB TAB TAB "writeln(obj[i].desc);\n");
    }
    fprintf(of,TAB TAB TAB "normaltxt();\n");
    fprintf(of,TAB TAB TAB "if(obj[i].position==WEARED){\n");
    fprintf(of,TAB TAB TAB TAB "writesameln(\"  \");\n");
    if(hardcoded_messages==false)
        fprintf(of,TAB TAB TAB TAB "show_messagenlf(1018);\n");
    else
        fprintf(of,TAB TAB TAB TAB "show_messagenlf(message1018);\n");
    fprintf(of,TAB TAB TAB "}\n");
    fprintf(of,TAB TAB TAB "writeln(\"\");\n");
    fprintf(of,TAB TAB "}\n");
    fprintf(of,TAB "}\n");
    //fprintf(of,TAB "normaltxt();\n");

    if(hardcoded_messages==false)
        fprintf(of,TAB "if(gs==0) show_message(1033);\n}\n\n");
    else
        fprintf(of,TAB "if(gs==0) show_message(message1033);\n}\n\n");

    fprintf(of, "unsigned int move(unsigned int dir)\n");
    fprintf(of, "{\n");
    fprintf(of, TAB
        "if(world[search_room(current_position)].directions[dir]!=0) {\n");
    fprintf(of, TAB TAB "next_position="
        "world[search_room(current_position)].directions[dir];\n");
    fprintf(of, TAB TAB "marker[120]=false;\n");
    fprintf(of, TAB TAB "return 1;\n");
    fprintf(of, TAB "} else \n");
    if(hardcoded_messages==false)
        fprintf(of, TAB TAB "show_message(1008);\n");
    else
        fprintf(of, TAB TAB "show_message(message1008);\n");
    fprintf(of, TAB "return 0;\n}\n\n");

    fprintf(of, "unsigned int get(unsigned int o)\n");
    fprintf(of, "{\n");
    fprintf(of, TAB "dummy=search_object(o);\n");
    fprintf(of, TAB "odummy=&obj[dummy];\n");

    fprintf(of, TAB "if(odummy->position!=current_position) {\n");
    if(hardcoded_messages==false)
        fprintf(of, TAB TAB "show_message(1006);\n");
    else
        fprintf(of, TAB TAB "show_message(message1006);\n");
    fprintf(of, TAB TAB "return 1;\n");
    /* Euh... should not be isnotmovable==true here??? */
    fprintf(of, TAB "} else if(odummy->isnotmovable==false) {\n");
    if(hardcoded_messages==false)
        fprintf(of, TAB TAB "show_message(1005);\n");
    else
        fprintf(of, TAB TAB "show_message(message1005);\n");
    fprintf(of, TAB TAB "return 1;\n");
    fprintf(of, TAB
        "} else if(counter[120]+odummy->weight>counter[122]){ \n");
    if(hardcoded_messages==false)
        fprintf(of, TAB TAB "show_message(1003);\n");
    else
        fprintf(of, TAB TAB "show_message(message1003);\n");
    fprintf(of, TAB TAB TAB "return 1;\n");
    fprintf(of, TAB "} else if(counter[124]+odummy->size>counter[121]) {\n");
    if(hardcoded_messages==false)
        fprintf(of, TAB TAB "show_message(1004);\n");
    else
        fprintf(of, TAB TAB "show_message(message1004);\n");
    fprintf(of, TAB TAB "return 1;\n");
    fprintf(of, TAB "} else {\n");
    fprintf(of, TAB TAB "odummy->position=CARRIED;\n");
    fprintf(of, TAB TAB "++counter[119];\n");
    fprintf(of, TAB TAB "counter[120]+=odummy->weight;\n");
    fprintf(of, TAB TAB "counter[124]+=odummy->size;\n");
    fprintf(of, TAB "}\n");
    fprintf(of, TAB "return 0;\n");
    fprintf(of, "}\n");

    /* Check among two verbs */
    fprintf(of, "boolean vov(unsigned int v1, unsigned int v2);\n");

    /* Check among two verbs AND a name */
    fprintf(of,
        "boolean vovn(unsigned int v1, unsigned int v2, unsigned int n);\n");

    /* Check among two nouns1 */
    fprintf(of, "boolean non1(unsigned int n1, unsigned int n2);\n");

    /* Check for a name and noun */
    fprintf(of, "boolean cvn(unsigned int v, unsigned int n);\n");
    
    /* Send all objects to a room in particular */
    fprintf(of, "void sendallroom(unsigned int s);\n");

    /* Get current position of an object */
    fprintf(of, "unsigned int get_object_position(unsigned int c)\n");
    fprintf(of, "{\n");
    fprintf(of, "    return obj[search_object(c)].position;\n");
    fprintf(of, "}\n");
    /* Check if an object is here */
    fprintf(of, "boolean object_is_here(unsigned int c)\n");
    fprintf(of, "{\n");
    fprintf(of,
        "    return obj[search_object(c)].position==current_position;\n");
    fprintf(of, "}\n");
    
    /* Check if an object is carried */
    fprintf(of, "boolean object_is_carried(unsigned int c)\n");
    fprintf(of, "{\n");
    fprintf(of,
        "    return obj[search_object(c)].position==CARRIED;\n");
    fprintf(of, "}\n");
    /* Check if an object is available */
    fprintf(of, "boolean object_is_available(unsigned int c)\n");
    fprintf(of, "{\n");
    fprintf(of,
        "    return object_is_here(c)||object_is_carried(c);\n");
    fprintf(of, "}\n");
    /* Set current position of an object */
    fprintf(of, "void set_object_position(unsigned int c, int pos)\n");
    fprintf(of, "{\n");
    fprintf(of, "    obj[search_object(c)].position=pos;\n");
    fprintf(of, "}\n");
    /* Bring here an object */
    fprintf(of, "void bring_object_here(unsigned int c)\n");
    fprintf(of, "{\n");
    fprintf(of, "    set_object_position(c,current_position);\n");
    fprintf(of, "}\n");

     /* Check for a position and marker */

    if(hardcoded_messages==false) {
        fprintf(of,
            "void amsm(unsigned int p, unsigned char c, boolean v,"
            " unsigned int m)\n");

    } else {
        fprintf(of,
            "void amsm(unsigned int p, unsigned char c, boolean v,"
            " char *m)\n");
    }

    fprintf(of, "{\n");
    fprintf(of,
        "    if(current_position==p&&marker[c]==v) show_message(m);\n");
    fprintf(of, "}\n");


    /* If a name and a noun and avai conditions are given */
    fprintf(of,
        "unsigned int cvna(unsigned int v, unsigned int n, unsigned int o);\n");

    fprintf(of, "void drop(unsigned int o)\n{\n");
    fprintf(of, TAB  "dummy=search_object(o);\n");
    fprintf(of, TAB  "odummy=&obj[dummy];\n");

    fprintf(of, TAB  "if(odummy->position==CARRIED){\n");
    fprintf(of, TAB TAB "odummy->position=current_position;\n");
    fprintf(of, TAB TAB "--counter[119];\n");
    fprintf(of, TAB TAB "counter[120]-=odummy->weight;\n");
    fprintf(of, TAB TAB "counter[124]-=odummy->size;\n");
    fprintf(of, TAB "} else\n");
    if(hardcoded_messages==false)
        fprintf(of, TAB TAB "show_message(1007);\n");
    else
        fprintf(of, TAB TAB "show_message(message1007);\n");
    fprintf(of, "}\n\n");

    fprintf(of, "void jump(unsigned int p)\n{\n");
    fprintf(of, TAB "next_position=p;\n");
    fprintf(of, TAB "marker[120]=false;\n");
    fprintf(of, "}\n\n");

    fprintf(of, "void hold(unsigned int p);\n");

}

/** Create the code for the dictionary in the output file.
*/
void output_dictionary(FILE *of, word* dictionary, unsigned int dsize)
{
    unsigned int i, size_d;
    fprintf(of, "#define DSIZE %d\n",dsize);

    /*  I did a few tests and it seemed to me that it is not worth compressing
        the dictionary. */
    fprintf(of, "word dictionary[DSIZE]={\n");
    for(i=0; i<dsize;++i) {
        fprintf(of, TAB "{\"%s\",%d,%d}",dictionary[i].w,
        dictionary[i].code,dictionary[i].t);

        if(i<dsize-1) {
            fprintf(of,",");
        }
        fprintf(of,"\n");
    }
    fprintf(of,"};\n\n");
}


/** Create the code for the rooms' description in the output file. */
void output_rooms(FILE *of, room* world, unsigned int rsize)
{
    unsigned int i,j;
    char *long_d;
    char *p;
    unsigned int size_d=0;
    if(compress_messages==true) {
        for(i=0; i<rsize;++i) {
            fprintf(of, "char long_d%d[]={",world[i].code);
            compress(of, encodechar(world[i].long_d));
            fprintf(of, "};\n");
            fprintf(of, "char s_desc%d[]={",world[i].code);
            compress(of, encodechar(world[i].s));
            fprintf(of, "};\n");
            fprintf(of, "char short_d%d[]={",world[i].code);
            compress(of, encodechar(world[i].short_d));
            fprintf(of, "};\n");
        }
    }
    fprintf(of, "#define RSIZE %d\n",rsize);
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
        }
        fprintf(of, ",");
        if(compress_messages==true) {
            fprintf(of, "s_desc%d",world[i].code);
        } else {
            fprintf(of, "\"%s\"", encodechar(world[i].s));
        }
        fprintf(of, ",");
        if(compress_messages==true) {
            fprintf(of, "short_d%d",world[i].code);
        } else {
            fprintf(of, "\"%s\"", world[i].short_d);
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
}

/** Create the code for the messages in the output file. */
void output_messages(FILE *of, message* msg, unsigned int msize)
{
    unsigned int i,j;
    unsigned int size_d=0;
    if(compress_messages==true||hardcoded_messages==true) {
        for(i=0; i<msize;++i) {
            fprintf(of, "char message%d[]=",msg[i].code);
            if(compress_messages==true) {
                fprintf(of,"{");
                compress(of, encodechar(msg[i].txt));
                fprintf(of,"}");
            } else {
                fprintf(of,"\"%s\"",encodechar(msg[i].txt));
            }
            fprintf(of, ";\n");

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
    fprintf(of, "#define OSIZE %d\n",osize);
    fprintf(of, "object obj[OSIZE]={\n");
    for(i=0; i<osize;++i) {
        if(compress_messages==true) {
                fprintf(of, TAB "{%d,\"%s\",desc_l%d,",obj[i].code,
                    obj[i].s,obj[i].code);
            } else {
                fprintf(of, TAB "{%d,\"%s\",\"%s\",",obj[i].code,obj[i].s,
                    encodechar(obj[i].desc));
            }
        fprintf(of, "%d,%d,%d,",obj[i].weight,obj[i].size,obj[i].position);
        if(obj[i].isnotmovable==true)
            fprintf(of, "true,");
        else
            fprintf(of, "false,");
        if(obj[i].isnotwereable==true)
            fprintf(of, "true}");
        else
            fprintf(of, "false}");

        if(i<osize-1)
            fprintf(of,",");

        fprintf(of,"\n");
    }
    fprintf(of,"};\n\n");
}

/** Write the code to provide the welcome message when the game starts. */
void output_greetings(FILE *f, info *header)
{
    fprintf(f, "\nvoid greetings(void)\n{\n");
    fprintf(f, TAB "evidence2();\n");
    fprintf(f, TAB "writeln(\"%s\\n\");\n", encodechar(header->name));
    fprintf(f, TAB "writesameln(\"%s\");\n", encodechar(header->author));
    fprintf(f, TAB "writeln(\"  %s\\n\");\n", encodechar(header->date));
    fprintf(f, TAB "writeln(\"%s\\n\");\n", encodechar(header->description));
    fprintf(f, TAB "writesameln(\"AWS \");\n");
    fprintf(f, TAB "writeln(\"%s\");\n", encodechar(header->version));
    fprintf(f, TAB "normaltxt();\n");
    fprintf(f, TAB "waitkey();\n");
    fprintf(f, "}\n");
}

/* Create code for HI conditions */
void output_hicond(FILE *f, char **cond, unsigned int size)
{
    unsigned int i;
    fprintf(f,"unsigned int hi_cond(void)\n{\n");
    for(i=0; i<size; ++i) {
        process_aws(f,cond[i]);
    }
    fprintf(f, TAB "return 0;\n");
    /*  Use of goto allows to spare a few bytes instead of putting a return 1
        every time in the code. The difference can be considerable in big
        adventures, as there are plenty of WAIT commands. */
    fprintf(f, TAB "return1: return 1;\n");
    fprintf(f,"}\n");
}

/* Create code for LOW conditions */
void output_lowcond(FILE *f, char **cond, unsigned int size)
{
    unsigned int i;
    fprintf(f,"unsigned int low_cond(void)\n{\n");
    for(i=0; i<size; ++i) {
        process_aws(f,cond[i]);
    }
    fprintf(f, TAB "return 0;\n");
    /*  Use of goto allows to spare a few bytes instead of putting a return 1
        every time in the code. The difference can be considerable in big
        adventures, as there are plenty of WAIT commands. */
    fprintf(f, TAB "return1: return 1;\n");

    fprintf(f,"}\n");
}

/* Create code for LOCAL conditions */
void output_local(FILE *f, localc* cond, unsigned int size)
{
    unsigned int i;
    unsigned int oldroom=-1;
    boolean first=true;
    fprintf(f,"unsigned int local_cond(void)\n{\n");
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

    fprintf(f, TAB "return 0;\n");
    /*  Use of goto allows to spare a few bytes instead of putting a return 1
        every time in the code. The difference can be considerable in big
        adventures, as there are plenty of WAIT commands. */
    fprintf(f, TAB "return1: return 1;\n");
    fprintf(f,"}\n");
}

/* Create code for the main game cycle */
void output_gamecycle(FILE *f)
{
    fprintf(f,"\nvoid game_cycle(void)\n{\n");
    fprintf(f, TAB "unsigned int k;\n");
    fprintf(f, TAB "boolean ve,pa;\n");
    fprintf(f, TAB "while(1){\n");
    fprintf(f, TAB TAB "current_position=next_position;\n");
    fprintf(f, TAB TAB
        "if(marker[120]==false&&(marker[121]==true||marker[122]==true)) {\n");
    if(add_clrscr==true)
        fprintf(f, TAB TAB TAB "clear();\n");
    else
        fprintf(f,TAB TAB TAB "writeln(\"\");\n");
    fprintf(f, TAB TAB TAB "evidence1();\n");
    if(compress_messages==true) {
        if(hardcoded_messages==true) {
            fprintf(f,TAB TAB TAB
                "show_messagenlf(world[search_room(current_position)]."
                "short_d);\n");
        } else {
            fprintf(f,TAB TAB TAB
                "write_textsl(world[search_room(current_position)]."
                "short_d);\n");
        }
    } else {
        fprintf(f, TAB TAB TAB
            "writesameln(world[search_room(current_position)].long_d);\n");
    }
    fprintf(f,TAB TAB TAB "writesameln(\"  \");\n");
    fprintf(f,TAB TAB TAB  "normaltxt();\n");

    if(compress_messages==true) {
        if(hardcoded_messages==true) {
            fprintf(f,TAB TAB TAB 
                "show_message(world[search_room(current_position)]."
                "long_d);\n");
        } else {
            fprintf(f,TAB TAB TAB
                "write_text(world[search_room(current_position)]."
                "long_d);\n");
        }
    } else {
        fprintf(f, TAB TAB TAB
            "writeln(world[search_room(current_position)].long_d);\n");
    }
    fprintf(f, TAB TAB TAB "writeln(\"\");\n");
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

    fprintf(f, TAB TAB TAB "if(marker[124]==true) {\n");
    fprintf(f, TAB TAB TAB TAB "pa=false;\n");
    if(use_6_directions)
        fprintf(f, TAB TAB TAB TAB "for(k=0; k<6; ++k)\n");
    else
        fprintf(f, TAB TAB TAB TAB "for(k=0; k<10; ++k)\n");

    fprintf(f, TAB TAB TAB TAB TAB
        "if(world[search_room(current_position)].directions[k]!=0) {\n");
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
    fprintf(f, TAB TAB TAB TAB TAB "writesameln(\" \");\n");
    fprintf(f, TAB TAB TAB TAB "}\n");
    fprintf(f, TAB TAB TAB TAB "normaltxt();\n");
    fprintf(f, TAB TAB TAB TAB "writeln(\"\");\n");
    fprintf(f, TAB TAB TAB "}\n");
    fprintf(f, TAB TAB "}\n");
    fprintf(f, TAB TAB "++counter[125];\n");
    fprintf(f, TAB TAB "--counter[126];\n");
    fprintf(f, TAB TAB "--counter[127];\n");
    fprintf(f, TAB TAB "--counter[128];\n");

    fprintf(f, TAB TAB "if(hi_cond()!=0) continue;\n");
    fprintf(f, TAB TAB "writeln(\"\");\n");

    if(hardcoded_messages==false)
        fprintf(f, TAB TAB "if(ls==0) show_message(1012);\n");
    else
        fprintf(f, TAB TAB "if(ls==0) "
            "show_message(message1012);\n");
    fprintf(f, TAB TAB "interrogationAndAnalysis(DSIZE);\n");
    fprintf(f, TAB TAB "if(local_cond()!=0) continue;\n");
    fprintf(f, TAB TAB "if(low_cond()!=0) continue;\n");
    if(hardcoded_messages==false)
        fprintf(f, TAB TAB "show_message(verb==0?1009:1010);\n");
    else
        fprintf(f, TAB TAB "show_message(verb==0?message1009:message1010);\n");
    fprintf(f, TAB "}\n");
    fprintf(f, "}\n");
}

/** Create the entry point of the game. */
void create_main(FILE *f, info *header)
{
    fprintf(f, "\nint main(void)\n{\n");
    fprintf(f, TAB "init_term();\n");
    fprintf(f, TAB "greetings();\n");
    fprintf(f, TAB "next_position=%d;\n",header->startroom);
    fprintf(f, TAB "marker[124]=true;\n");
    fprintf(f, TAB "marker[121]=true;\n");
    if(header->maxcarryingw==0) {
        header->maxcarryingw=10000;
    }
    if(header->maxcarryings==0) {
        header->maxcarryings=10000;
    }
    fprintf(f, TAB "counter[121]=%d;\n",header->maxcarryings);
    fprintf(f, TAB "counter[122]=%d;\n",header->maxcarryingw);
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
           " -v  or --version print version and exit\n");

    printf("\n");
}

/* Process options from the command line. */
unsigned int process_options(char *arg, char *name)
{
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
    } else if (strcmp(arg, "-v")==0 || strcmp(arg, "--version")==0) {
        printf("This is AWS2C version %s\n", VERSION);
        exit(0);
    }
    return 0;
}

/* Entry point of the program. */
int main(int argc, char **argv)
{
    FILE *f;
    FILE *of;
    unsigned int dsize;
    unsigned int rsize;
    unsigned int osize;
    unsigned int msize;
    unsigned int i;
    info *header;
    word* dictionary;
    room* world;
    object* objects;
    message* msg;
    char **hicond;
    unsigned int hicondsize;
    char **lowcond;
    unsigned int lowcondsize;
    localc* localcond;
    unsigned int localcondsize;
    unsigned int argumentr=1;

    init_analysis();
    while (argumentr<argc && process_options(argv[argumentr], argv[0])!=0) {
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
    if ((header=read_header(f))==NULL)
        exit(1);

    printf("done\n");

    printf("Read number of CONDIZIONIHI: ");
    hicondsize=get_hi_cond_size(f);
    printf("%d\n",hicondsize);
    printf("Read CONDIZIONIHI: ");
    if((hicond=read_cond(f, hicondsize))==NULL)
        exit(1);

    printf("done\n");

    printf("Read number of CONDIZIONILOW: ");
    lowcondsize=get_low_cond_size(f);
    printf("%d\n",lowcondsize);
    printf("Read CONDIZIONILOW: ");
    if((lowcond=read_cond(f, lowcondsize))==NULL)
        exit(1);
    printf("done\n");

    printf("Read number of CONDIZIONILOCALI: ");
    localcondsize=get_local_cond_size(f);
    printf("%d\n",localcondsize);
    printf("Read CONDIZIONILOCALI: ");
    if((localcond=read_local(f, localcondsize))==NULL)
        exit(1);
    printf("done\n");

    printf("Get dictionary size: ");
    dsize=get_dict_size(f);
    printf("%d\n",dsize);
    printf("Read dictionary: ");
    if((dictionary=read_dictionary(f,dsize))==NULL)
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
    if((objects=read_objects(f, osize))==NULL)
        exit(1);
    fclose(f);
    printf("done\n");

    printf("Create the output file\n");
    of=fopen(argv[argumentr+1],"w");
    if(of==NULL) {
        printf("Could not create output file.\n");
        return 1;
    }
    no_of_errors=0;
    output_header(of);
    if(compress_messages==true || compress_descriptions==true) {
        output_decoder(of);
    }
    output_dictionary(of, dictionary, dsize);
    output_rooms(of, world, rsize);
    output_messages(of, msg, msize);
    output_objects(of, objects, osize);
    output_utility_func(of);
    output_greetings(of, header);
    output_hicond(of, hicond, hicondsize);
    output_lowcond(of, lowcond, lowcondsize);
    output_local(of,localcond, localcondsize);
    output_gamecycle(of);
    create_main(of, header);
    output_optional_func(of);
    fclose(of);
    printf("File %s created\n",argv[argumentr+1]);
    printf("No of critical errors: %d\n",no_of_errors);
    if(no_of_errors>0)
        return 1;

    return 0;
}