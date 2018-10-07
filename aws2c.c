/* AWS to C converter 1.0 by Davide Bucci */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"aws.h"

#define BUFFERSIZE 1024
char buffer[BUFFERSIZE];
char function_res[BUFFERSIZE];
#define TAB "    "


#define start_function() function_res[0]='\0';

typedef struct localc_t {
    int room;
    char *condition;
} localc;

/** Read the dictionary contained in the file. The number of words to be read
    should have been already found.
    @return a pointer to the allocated dictionary, or NULL if something bad
        happened.
*/
word *read_dictionary(FILE *f, int size)
{
    int cw;
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
        } else {
            printf("Unknown word type.\n");
            free(dictionary);
            return NULL;
        }
    }
    return dictionary;
}

/** Get the dictionary size.
*/
int get_dict_size(FILE *f)
{
    fpos_t pos;
    int counter=0;

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
    int sl;
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
int get_room_number(FILE *f)
{
    fpos_t pos;
    int counter=0;
    int sl=0;
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
char **read_cond(FILE*f, int size)
{
    int i;
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
localc* read_local(FILE*f, int size)
{
    int i,r;
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
        getlinep(f);
        getlinep(f);
        sscanf(buffer, "%d",&(world[i].code));
        getlinep(f);
        world[i].long_d=calloc(strlen(buffer)+1,sizeof(char));
        strcpy(world[i].long_d,buffer);
        getlinep(f);
        world[i].s=calloc(strlen(buffer)+1,sizeof(char));
        strcpy(world[i].s,buffer);
        getlinep(f);
        world[i].short_d=calloc(strlen(buffer)+1,sizeof(char));
        strcpy(world[i].short_d,buffer);
        for(j=0;j<10;++j) {
            if(fscanf(f,"%d",&(world[i].directions[j]))!=1) {
                printf("Error reading directions.\n");
                free(world);
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
    int i,j;
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

        getlinep(f);
        sscanf(buffer, "%d",&(obj[i].weight));

        getlinep(f);
        sscanf(buffer, "%d",&(obj[i].inc1));

        getlinep(f);
        sscanf(buffer, "%d",&(obj[i].position));

        getlinep(f);
        if(strcmp(buffer,"FALSE")) {
            obj[i].flag1=false;
        } else {
            obj[i].flag1=true;
        }

        getlinep(f);
        if(strcmp(buffer,"FALSE")) {
            obj[i].flag2=false;
        } else {
            obj[i].flag2=true;
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
/** Change and encode characters that may create troubles when output, such
    as ".
    Exploits buffer.
*/
char *encodechar(char *input)
{
    int i,k;
    char c;
    for(i=0; (c=input[i])!='\0' && i<BUFFERSIZE-1;++i) {
        if(c=='\"') {
            buffer[k++]='\\';
        }
        buffer[k++]=c;
    }
    buffer[k++]='\0';
    return buffer;
}

/** Get the messages in the game
*/
message* read_messages(FILE *f, int size)
{
    char *errorp="Could not allocate enough memory for messages.\n";
    int i,j;
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
    }
    return msg;
}

/** Get the number of the objects in the game.
*/
int get_objects_number(FILE *f)
{
    fpos_t pos;
    int counter=0;
    int sl=0;
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
int get_messages_number(FILE *f)
{
    fpos_t pos;
    int counter=0;
    int sl=0;
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
int get_hi_cond_size(FILE *f)
{
    fpos_t pos;
    int counter=0;
    int sl=0;
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
int get_low_cond_size(FILE *f)
{
    fpos_t pos;
    int counter=0;
    int sl=0;
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
int get_local_cond_size(FILE *f)
{
    fpos_t pos;
    int counter=0;
    int sl=0;
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


/** Create the header of the output file.
*/
void create_header(FILE *of)
{
    fprintf(of,"/* Adventure Writing System, generated file */\n\n");
    fprintf(of,"#include<stdio.h>\n");
    fprintf(of,"#include<stdlib.h>\n");
    fprintf(of,"#include\"aws.h\"\n\n");
    fprintf(of,"#include\"inout.h\"\n");
    fprintf(of,"#include\"modern.h\"\n\n");
    fprintf(of,"extern int verb;\nextern int noun1;\nextern int noun2;\n"
        "extern int adve;\n");
    fprintf(of, "int dummy;\n");
    fprintf(of,"\n");

}

void output_utility_func(FILE *of)
{
    fprintf(of,"int current_position;\n");
    fprintf(of,"boolean marker[129];\n");
    fprintf(of,"int counter[129];\n");

    fprintf(of,"int search_object(int o)\n{\n");
    fprintf(of,TAB "int i;\n");
    fprintf(of,TAB "for(i=0; i<OSIZE;++i)\n");
    fprintf(of,TAB TAB "if(obj[i].code==o)\n");
    fprintf(of,TAB TAB TAB "return i;\n");
    fprintf(of,TAB "return -2;\n}\n\n");

    fprintf(of,"int search_room(int r)\n{\n");
    fprintf(of,TAB "int i;\n");
    fprintf(of,TAB "for(i=0; i<RSIZE;++i)\n");
    fprintf(of,TAB TAB "if(world[i].code==r)\n");
    fprintf(of,TAB TAB TAB "return i;\n");
    fprintf(of,TAB "return -2;\n}\n\n");

    fprintf(of,"boolean carry_object(int o)\n{\n");
    fprintf(of,TAB "int i;\n");
    fprintf(of,TAB "for(i=0; i<OSIZE;++i)\n");
    fprintf(of,TAB TAB "if(obj[i].code==o && obj[i].position==-1)\n");
    fprintf(of,TAB TAB TAB "return true;\n");
    fprintf(of,TAB "return false;\n}\n\n");
    
    fprintf(of,"void show_message(int m)\n{\n");
    fprintf(of,TAB "int i;\n");
    fprintf(of,TAB "for(i=0; i<MSIZE;++i)\n");
    fprintf(of,TAB TAB "if(msg[i].code==m)\n");
    fprintf(of,TAB TAB TAB "writeln(msg[i].txt);\n");
    fprintf(of, "}\n\n");

    fprintf(of,"void show_messagenlf(int m)\n{\n");
    fprintf(of,TAB "int i;\n");
    fprintf(of,TAB "for(i=0; i<MSIZE;++i)\n");
    fprintf(of,TAB TAB "if(msg[i].code==m)\n");
    fprintf(of,TAB TAB TAB "writesameln(msg[i].txt);\n");
    fprintf(of, "}\n\n");


    fprintf(of,"void inventory(void)\n{\n");
    fprintf(of,TAB "int i, gs=0;\n");
    fprintf(of,TAB "show_message(1032);\n");
    fprintf(of,TAB "for(i = 0; i<OSIZE; ++i) {\n");
    fprintf(of,TAB TAB "if(obj[i].position==-1) {\n");
    fprintf(of,TAB TAB TAB "++gs;\n");
    fprintf(of,TAB TAB TAB "writeln(obj[i].desc);\n");
    fprintf(of,TAB TAB "}\n");
    fprintf(of,TAB "}\n");
    fprintf(of,TAB "if(gs==0) show_message(1033);\n}\n\n");
}

/** Create the code for the dictionary in the output file.
*/
void output_dictionary(FILE *of, word* dictionary, int dsize)
{
    int i;
    fprintf(of, "#define DSIZE %d\n",dsize);

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


/** Create the code for the dictionary in the output file.
*/
void output_rooms(FILE *of, room* world, int rsize)
{
    int i,j;
    fprintf(of, "#define RSIZE %d\n",rsize);
    fprintf(of, "room world[RSIZE]={\n");
    for(i=0; i<rsize;++i) {
        fprintf(of, TAB "{%d,\"%s\",\"%s\",\"%s\",",
            world[i].code, world[i].long_d, world[i].s, world[i].short_d);
        fprintf(of, "{");
        for(j=0; j<9;++j) {
            fprintf(of, "%d,", world[i].directions[j]);
        }
        fprintf(of, "%d}", world[i].directions[j]);
        fprintf(of,"}");
        if(i<rsize-1) {
            fprintf(of,",");
        }
        fprintf(of,"\n");
    }
    fprintf(of,"};\n\n");
}

/** Create the code for the messages in the output file.
*/
void output_messages(FILE *of, message* msg, int msize)
{
    int i,j;
    fprintf(of, "#define MSIZE %d\n",msize);
    fprintf(of, "message msg[MSIZE]={\n");
    for(i=0; i<msize;++i) {
        fprintf(of, TAB "{%d,\"%s\"}", msg[i].code, msg[i].txt);
        if(i<msize-1) {
            fprintf(of,",");
        }
        fprintf(of,"\n");
    }
    fprintf(of,"};\n\n");
}

/** Create the code for the objects in the output file.
*/
void output_objects(FILE *of, object* obj, int osize)
{
    int i,j;
    fprintf(of, "#define OSIZE %d\n",osize);
    fprintf(of, "object obj[OSIZE]={\n");
    for(i=0; i<osize;++i) {
        fprintf(of, TAB "{%d,\"%s\",\"%s\",",obj[i].code,obj[i].s,obj[i].desc);
        fprintf(of, "%d,%d,%d,",obj[i].weight,obj[i].inc1,obj[i].position);
        if(obj[i].flag1==true)
            fprintf(of, "true,");
        else
            fprintf(of, "false,");
        if(obj[i].flag2==true)
            fprintf(of, "true}");
        else
            fprintf(of, "false}");

        if(i<osize-1)
            fprintf(of,",");

        fprintf(of,"\n");
    }
    fprintf(of,"};\n\n");
}

/** Write the code to provide the welcome message when the game starts.
*/
void output_greetings(FILE *f, info *header)
{
    fprintf(f, "\nvoid greetings(void)\n{\n");
    fprintf(f, TAB "writeln(\"%s\");\n", encodechar(header->name));
    fprintf(f, TAB "writeln(\"%s\");\n", encodechar(header->author));
    fprintf(f, TAB "writeln(\"%s\");\n", encodechar(header->date));
    fprintf(f, TAB "writeln(\"%s\");\n", encodechar(header->description));
    fprintf(f, TAB "writeln(\"AWS version %s\");\n",
        encodechar(header->version));
    fprintf(f, "}\n");
}

char token[BUFFERSIZE];

/** Get a new token (store it in the shared variable "token") and
    return the new position in the line.
*/
int get_token(char *line, int pos)
{
    char c;
    int k=0;
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

void strcon(char *str1, const char* str2)
{
    int i,j;
    char c;

    for(i=0;str1[i]!='\0'&&i<BUFFERSIZE;++i) ;

    for(j=0;(c=str2[j])!='\0'&&i<BUFFERSIZE;++j)
        str1[i++]=str2[j];
    
    str1[i]='\0';
}

int process_functions(char *line, int scanpos)
{
    int value;
    scanpos=get_token(line, scanpos);
    if(strcmp(token,"NO1")==0) {
        strcon(function_res,"noun1");
    } else if(strcmp(token,"CTR")==0) {
        strcon(function_res,"counter[");
        scanpos=process_functions(line,scanpos);
        strcon(function_res,"]");
    } else if(strcmp(token,"ROOM")==0) {
        strcon(function_res,"current_position");
    } else {
        if(sscanf(token, "%d",&value)==1) {
            sprintf(token, "%d", value);
            strcon(function_res,token);
        } else {
            printf("Function not recognized %s\n", token);
        }
        
    }
    return scanpos;
}


/*  Decisions */

/** AT */
int decision_at(FILE *f, char *line, int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "current_position==%s", function_res);
    return scanpos;
}
/** NOTAT */
int decision_notat(FILE *f, char *line, int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "current_position!=%s", function_res);
    return scanpos;
}

/** SET? */
int decision_set(FILE *f, char *line, int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "marker[%s]==true", function_res);
    return scanpos;
}

/** RES? */
int decision_res(FILE *f, char *line, int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "marker[%s]==false", function_res);
    return scanpos;
}
/** ROOMGT */
int decision_roomgt(FILE *f, char *line, int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "current_position>%s", function_res);
    return scanpos;
}
/** ROOMLT */
int decision_roomlt(FILE *f, char *line, int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "current_position<%s", function_res);
    return scanpos;
}
/** CARR */
int decision_carr(FILE *f, char *line, int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "carry_object(%s)==true", function_res);
    return scanpos;
}
/** NOTIN */        // TODO implement functions
int decision_notin(FILE *f, char *line, int scanpos)
{
    int position,object;
    scanpos=get_token(line, scanpos);
    sscanf(token, "%d", &object);
    scanpos=get_token(line, scanpos);
    sscanf(token, "%d", &position);
    fprintf(f, "obj[search_object(%d)].position!=%d", object,position);
    return scanpos;
}
/** EQU? */     // TODO implement functions
int decision_equ(FILE *f, char *line, int scanpos)
{
    int counter,value;
    scanpos=get_token(line, scanpos);
    sscanf(token, "%d", &counter);
    scanpos=get_token(line, scanpos);
    sscanf(token, "%d", &value);
    fprintf(f, "counter[%d]==%d", counter,value);
    return scanpos;
}
/** VERB */
int decision_verb(FILE *f, char *line, int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "verb==%s", function_res);
    return scanpos;
}
/** NOUN */
int decision_noun(FILE *f, char *line, int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "(noun1==%s||noun2==%s)", function_res, function_res);
    return scanpos;
}
/** AVAI */
int decision_avai(FILE *f, char *line, int scanpos)
{
    int counter,value;
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "((obj[search_object(%s)].position==current_position)||", 
        function_res);
    fprintf(f, "(obj[search_object(%s)].position==-1))", function_res);

    return scanpos;
}
/** NO2EQ */
int decision_no2eq(FILE *f, char *line, int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "noun2==%s", function_res);
    return scanpos;
}
/** NOTCARR */
int decision_notcarr(FILE *f, char *line, int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "obj[search_object(%s)].position!=-1", function_res);
    return scanpos;
}
/** NO1GT */
int decision_no1gt(FILE *f, char *line, int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "noun1>%s", function_res);
    return scanpos;
}
/** NO1LT */
int decision_no1lt(FILE *f, char *line, int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "(noun1>0&&noun1<%s)", function_res);
    return scanpos;
}
/** NO2GT */
int decision_no2gt(FILE *f, char *line, int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, "noun2>%s", function_res);
    return scanpos;
}
/** CTRGT */        // TODO implement functions
int decision_ctrgt(FILE *f, char *line, int scanpos)
{
    int counter,value;
    scanpos=get_token(line, scanpos);
    sscanf(token, "%d", &counter);
    scanpos=get_token(line, scanpos);
    sscanf(token, "%d", &value);
    fprintf(f, "counter[%d]>%d", counter,value);
    return scanpos;
}
/** IN */       // TODO implement functions
int decision_in(FILE *f, char *line, int scanpos)
{
    int object,position;
    scanpos=get_token(line, scanpos);
    sscanf(token, "%d", &object);
    scanpos=get_token(line, scanpos);
    sscanf(token, "%d", &position);
    fprintf(f, "obj[search_object(%d)].position==%d", object, position);
    return scanpos;
}



/* Actions */

/** PRESSKEY */
int action_presskey(FILE *f, char *line, int scanpos)
{
    fprintf(f, TAB TAB "printf(\"press [return]\");\n");
    fprintf(f, TAB TAB "getchar();\n");
    return scanpos;
}

/** GOTO */
int action_goto(FILE *f, char *line, int scanpos)
{
    int position;
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, TAB TAB "current_position=%s;\n", function_res);
    fprintf(f, TAB TAB "marker[120]=false;\n");

    return scanpos;
}

/** SET */
int action_set(FILE *f, char *line, int scanpos)
{
    int position;
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, TAB TAB "marker[%s]=true;\n", function_res);
    return scanpos;
}

/** RESE */
int action_rese(FILE *f, char *line, int scanpos)
{
    int position;
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, TAB TAB "marker[%s]=false;\n", function_res);
    return scanpos;
}

/** CSET */   // TODO accept functions
int action_cset(FILE *f, char *line, int scanpos)
{
    int position, value;
    scanpos=get_token(line, scanpos);
    sscanf(token, "%d", &position);
    scanpos=get_token(line, scanpos);
    sscanf(token, "%d", &value);
    fprintf(f, TAB TAB "counter[%d]=%d;\n", position, value);
    return scanpos;
}

/** INCR */
int action_incr(FILE *f, char *line, int scanpos)
{
    int position, value;
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, TAB TAB "++counter[%s];\n", function_res);
    return scanpos;
}

/** DECR */
int action_decr(FILE *f, char *line, int scanpos)
{
    int position, value;
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, TAB TAB "if(counter[%s]>0) --counter[%s];\n", function_res, 
        function_res);
    return scanpos;
}

/** MESS */
int action_mess(FILE *f, char *line, int scanpos)
{
    int position, value;
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, TAB TAB "show_message(%s);\n",  function_res);
    return scanpos;
}
/** MESSNOLF */
int action_messnolf(FILE *f, char *line, int scanpos)
{
    int position, value;
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, TAB TAB "show_messagenlf(%s);\n",  function_res);
    return scanpos;
}

/** BRIN */
int action_brin(FILE *f, char *line, int scanpos)
{
    int position, value;
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, TAB TAB "obj[search_object(%s)].position=current_position;\n", 
        function_res);
    return scanpos;
}

/** QUIT */
int action_exit(FILE *f, char *line, int scanpos)
{
    
    fprintf(f, TAB TAB "leave(); exit(0);\n");
    return scanpos;
}
/** EXIT */
int action_quit(FILE *f, char *line, int scanpos)
{
    fprintf(f, TAB TAB "leave(); exit(0);\n");
    return scanpos;
}
/** INVE */
int action_inve(FILE *f, char *line, int scanpos)
{
    fprintf(f, TAB TAB "inventory();\n");
    return scanpos;
}
/** move */
int action_move(FILE *f, char *line, int scanpos, int dir)
{
    int position, value;
    fprintf(f, TAB TAB 
        "if(world[search_room(current_position)].directions[%d]!=0) {\n",dir-1);
    fprintf(f, TAB TAB TAB "current_position="
        "world[search_room(current_position)].directions[%d];\n", dir-1);
    fprintf(f, TAB TAB TAB "marker[120]=false;\n");
    fprintf(f, TAB TAB "} else \n");
    fprintf(f, TAB TAB TAB "show_message(1008);\n");

    return scanpos;
}
/** LOOK */
int action_look(FILE *f, char *line, int scanpos)
{
    fprintf(f, TAB TAB "marker[120]=false;\n");
    return scanpos;
}
/** DROP */
int action_drop(FILE *f, char *line, int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, TAB TAB "if(obj[search_object(%s)].position==-1){\n",
        function_res);
    fprintf(f, TAB TAB TAB 
        "obj[search_object(%s)].position=current_position;\n", function_res);
    fprintf(f, TAB TAB "} else\n");
    fprintf(f, TAB TAB TAB "show_message(1007);\n");
    return scanpos;
}
/** TO */       // TODO accept functions
int action_to(FILE *f, char *line, int scanpos)
{
    int position, object;
    scanpos=get_token(line, scanpos);
    sscanf(token, "%d", &object);
    scanpos=get_token(line, scanpos);
    sscanf(token, "%d", &position);
    fprintf(f, TAB TAB "obj[search_object(%d)].position=%d;\n", object,
        position);
    return scanpos;
}
/** OKAY */
int action_okay(FILE *f, char *line, int scanpos)
{
    fprintf(f, TAB TAB "show_message(1000);\n");
    fprintf(f, TAB TAB "return 1;\n");
    return scanpos;
}
/** PRIN */
int action_prin(FILE *f, char *line, int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, TAB TAB "printf(\"%%d\\n\",%s);\n",function_res);
    return scanpos;
}
/** PRINNOLF */
int action_prinnolf(FILE *f, char *line, int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, TAB TAB "printf(\"%%d\",%s);\n",function_res);
    return scanpos;
}
/** ADDC */     // TODO accept functions
int action_addc(FILE *f, char *line, int scanpos)
{
    int counter, value;
    scanpos=get_token(line, scanpos);
    sscanf(token, "%d", &counter);
    scanpos=get_token(line, scanpos);
    sscanf(token, "%d", &value);
    fprintf(f, TAB TAB "counter[%d]+=%d;\n", counter, value);
    return scanpos;
}
/** HOLD */
int action_hold(FILE *f, char *line, int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, TAB TAB "for(dummy=0; dummy<%s; ++dummy) wait1s();\n",
        function_res);
    return scanpos;
}
/** GET */
int action_get(FILE *f, char *line, int scanpos)
{
    start_function();
    scanpos=process_functions(line, scanpos);
    fprintf(f, TAB TAB 
        "if(obj[search_object(%s)].position==current_position){\n",
            function_res);
    fprintf(f, TAB TAB TAB 
        "obj[search_object(%s)].position=-1;\n", function_res);
    fprintf(f, TAB TAB "} else\n");
    fprintf(f, TAB TAB TAB "show_message(1007);\n");
    return scanpos;
}
/** SETCONN */       // TODO accept functions
int action_setconn(FILE *f, char *line, int scanpos)
{
    int room1, direction, room2;
    scanpos=get_token(line, scanpos);
    sscanf(token, "%d", &room1);
    scanpos=get_token(line, scanpos);
    sscanf(token, "%d", &direction);
    scanpos=get_token(line, scanpos);
    sscanf(token, "%d", &room2);
    fprintf(f, TAB TAB "world[search_room(%d)].directions[%d]=%d;\n",
        room1, direction-1, room2);
    return scanpos;
}
/** SWAP */      // TODO accept functions
int action_swap(FILE *f, char *line, int scanpos)
{
    int object1, object2;
    scanpos=get_token(line, scanpos);
    sscanf(token, "%d", &object1);
    scanpos=get_token(line, scanpos);
    sscanf(token, "%d", &object2);
    fprintf(f, TAB TAB "dummy=obj[search_object(%d)].position;\n",object1);
    fprintf(f, TAB TAB 
        "obj[search_object(%d)].position=obj[search_object(%d)].position;\n",
        object1,object2);
    fprintf(f, TAB TAB "obj[search_object(%d)].position=dummy;\n",object2);
    return scanpos;
}
/** WAIT */
int action_wait(FILE *f, char *line, int scanpos)
{
    fprintf(f, TAB TAB "return 1;\n");
    return scanpos;
}

/** Main processing function. Exploits buffer.
*/
void process_aws(FILE *f, char *line)
{
    int scanpos=0;
    scanpos=get_token(line, scanpos);
    //printf("line [%s], token %s\n", line, token);
    
    if(strcmp(token, "IF")!=0) {
        printf("Unrecognised start of aws condition %s instead of IF.\n",
            token);
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
        } else if(strcmp(token,"NOUN")==0) {
            scanpos=decision_noun(f, line, scanpos);
        } else if(strcmp(token,"AVAI")==0) {
            scanpos=decision_avai(f, line, scanpos);
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
        } else if(strcmp(token,"OR")==0) {
            fprintf(f,"||");
        } else if(strcmp(token,"AND")==0) {
            // this is a trick to have the behaviour of AND as it is in AWS.
            fprintf(f,") if("); // There, AND has the same priority of OR :-(
        } else if(strcmp(token,"THEN")==0) {
            break;
        } else {
            printf("Unrecognised decision %s in [%s].\n", token, line);
            return;
        }
    }
    
    fprintf(f,") {\n");  // ACTIONS
    while(1) {
        scanpos=get_token(line, scanpos);
        if(strcmp(token,"PRESSKEY")==0) {
            scanpos=action_presskey(f, line, scanpos);
        } else if(strcmp(token,"GOTO")==0) {
            scanpos=action_goto(f, line, scanpos); 
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
        } else if(strcmp(token,"EXIT")==0) {
            scanpos=action_exit(f, line, scanpos);
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
        } else {
            printf("Unrecognised action %s in [%s].\n", token, line);
            return;
        }
    }
    fprintf(f, TAB "}\n\n");
}

void output_hicond(FILE *f, char **cond, int size)
{
    int i;
    fprintf(f,"int hi_cond(void)\n{\n");
    for(i=0; i<size; ++i) {
        process_aws(f,cond[i]);
    }
    fprintf(f, TAB "return 0;\n");
    fprintf(f,"}\n");
}

void output_lowcond(FILE *f, char **cond, int size)
{
    int i;
    fprintf(f,"int low_cond(void)\n{\n");
    for(i=0; i<size; ++i) {
        process_aws(f,cond[i]);
    }
    fprintf(f, TAB "return 0;\n");
    fprintf(f,"}\n");
}

void output_local(FILE *f, localc* cond, int size)
{
    int i;
    fprintf(f,"int local_cond(void)\n{\n");
    for(i=0; i<size; ++i) {
        fprintf(f, TAB "if(current_position==%d) {\n", cond[i].room);
        process_aws(f,cond[i].condition);
        fprintf(f, TAB "}\n");
    }
    
    fprintf(f, TAB "return 0;\n");
    fprintf(f,"}\n");
}


void output_gamecycle(FILE *f)
{
    fprintf(f,"\nvoid game_cycle(void)\n{\n");
    fprintf(f, TAB "int k;\n");
    fprintf(f, TAB "while(1){\n");
    fprintf(f, TAB TAB 
        "if(marker[120]==false&&(marker[121]==true||marker[122]==true)) {\n");
    fprintf(f, TAB TAB TAB 
        "writeln(world[search_room(current_position)].long_d);\n");
    fprintf(f, TAB TAB TAB "marker[120]=true;\n");
    fprintf(f, TAB TAB TAB "show_message(1031);\n");
    fprintf(f, TAB TAB TAB "for(k=0;k<OSIZE;++k)\n");
    fprintf(f, TAB TAB TAB TAB 
        "if(obj[k].position==current_position) writeln(obj[k].desc);\n");
    fprintf(f, TAB TAB "\n");
    fprintf(f, TAB TAB "if(marker[124]==true) {\n");
    fprintf(f, TAB TAB TAB "show_messagenlf(1020);\n");
    fprintf(f, TAB TAB TAB "for(k=0; k<10; ++k)\n");
    fprintf(f, TAB TAB TAB TAB 
        "if(world[search_room(current_position)].directions[k]!=0) {\n");
    fprintf(f, TAB TAB TAB TAB TAB "show_messagenlf(1021+k);\n");
    fprintf(f, TAB TAB TAB TAB TAB "writesameln(\" \");\n");
    fprintf(f, TAB TAB TAB TAB "}\n");
    fprintf(f, TAB TAB TAB TAB "writeln(\"\");\n");
    fprintf(f, TAB TAB "}}\n");
    fprintf(f, TAB TAB "++counter[125];\n");
    fprintf(f, TAB TAB "--counter[126];\n");
    fprintf(f, TAB TAB "--counter[127];\n");
    fprintf(f, TAB TAB "--counter[128];\n");
    
    fprintf(f, TAB TAB "hi_cond();\n");
    fprintf(f, TAB TAB "show_message(1012);\n");
    fprintf(f, TAB TAB "interrogationAndAnalysis(DSIZE);\n");
    fprintf(f, TAB TAB "if(local_cond()!=0) continue;\n");
    fprintf(f, TAB TAB "if(low_cond()!=0) continue;\n");
    fprintf(f, TAB TAB "show_message(verb==0?1009:1010);\n");
    fprintf(f, TAB "}\n");
    fprintf(f, "}\n");
}

/** Create the entry point of the game.
*/
void create_main(FILE *f, info *header)
{
    fprintf(f, "\nint main(void)\n{\n");
    fprintf(f, TAB "normaltxt();\n");
    fprintf(f, TAB "greetings();\n");
    fprintf(f, TAB "current_position=%d;\n",header->startroom);
    fprintf(f, TAB "marker[124]=true;\n");
    fprintf(f, TAB "game_cycle();\n");
    fprintf(f, TAB "return 0;\n");
    fprintf(f, "}\n");
}


int main(int argc, char **argv)
{
    if(argc<3) {
        printf("Adventure Writing System to C compiler, version 1.0\n");
        printf("Davide Bucci 2018\n\n");
        printf("The number of arguments is not correct\n\n");
        printf("Usage: %s inputfile.aws outputfile\n\n",argv[0]);
        printf("then compile with file inout.c\n");
        exit(0);
    }
    printf("Reading %s\n",argv[1]);
    FILE *f=fopen(argv[1],"r");
    FILE *of;
    int dsize;
    int rsize;
    int osize;
    int msize;
    int i;
    info *header;
    word* dictionary;
    room* world;
    object* objects;
    message* msg;
    char **hicond;
    int hicondsize;
    char **lowcond;
    int lowcondsize;
    localc* localcond;
    int localcondsize;
    

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
    of=fopen(argv[2],"w");
    if(of==NULL) {
        printf("Could not create output file.\n");
        return 1;
    }
    create_header(of);
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
    fclose(of);
    printf("File %s created\n",argv[2]);
}