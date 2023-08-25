/* A simple Huffman compression tool. D. Bucci 2018 */

#include <stdio.h>
#include <stdlib.h>
#include "compress.h"

int pointer;
int bitv;
char *comp;

typedef enum boolean_t {true=1, false=0} boolean;
int num_nodes;

compression stats[256];
unsigned int frequencies[256];
unsigned int code[256];
unsigned int length[256];
long numofchar;
char codestack[20];
int  valuestack;
int stackp;
boolean show_codes=true;
int max_len;
node treearray[256];

/* Analyse the frequency of occurrence of each letter in the given text */
void analyze(char *text)
{
    int i=0;
    unsigned char c;

    do {
        c=text[i++];
        ++stats[c].occur;
        ++numofchar;
    } while(c!='\0');
    if (i>max_len)
        max_len=i;
}

/* Get the maximum length of the analysed strings. */
int get_max_len(void)
{
    return max_len;
}

/* Useful for the sort function. */
int compare(const void *a1, const void *a2)
{
    return ((compression *)a1)->occur-((compression*)a2)->occur;
}

/* Push operation a stack used to put together the code during compression. */
void push(char c)
{
    codestack[stackp++]=c;
    valuestack<<=1;
    if(c=='1')
        ++valuestack;
}

/* Pop operation a stack used to put together the code during compression. */
char pop(void)
{
    char c=codestack[--stackp];
    codestack[stackp]='\0';
    valuestack>>=1;
    return c;
}

/* Depth-first exploration of the tree.*/
void explore_tree(node *n)
{
    int c,i;
    treearray[n->nodeindex]=*n;

    if(n->son0==NULL&&n->son1==NULL) {
        c=n->c;
        code[n->c]=valuestack;
        length[n->c]=stackp;
        if(show_codes) {
            /* It is a leaf! */
            if(c<0)
                printf("       %18s   Error!\n", codestack);
            else {
                if (c=='\n'||c=='\0')
                    c='\\';
                printf("%8d %6.2f%%, code: ",frequencies[n->c],
                    (float)frequencies[n->c]/numofchar*100);
                for(i=18; i>=0; --i) {
                    if(i<length[n->c])
                        printf("%c",codestack[i]);
                    else
                        printf(" ");
                }
                printf(" [%c", c);
                if (n->c=='\n')
                    printf("n");
                 if (n->c=='\0')
                    printf("0");
                printf("] i.e. 0x%X (%d bits)\n",
                    (int)n->c,length[n->c]);
            }
        }
    } else {
        if(n->son0!=NULL) {
            push('0');
            explore_tree(n->son0);
        } else if(n->son1!=NULL) {
            push('1');
            explore_tree(n->son1);
        }
        if(n->son0!=NULL&&n->son1!=NULL) {
            push('1');
            explore_tree(n->son1);
        }
    }
    pop();
}

/* Create the Huffman tree. */
node *create_tree(void)
{
    node *np0, *np1, *np;
    boolean created;
    int i;
    num_nodes=0;

    for(i=0; i<256;++i)
        frequencies[i]=stats[i].occur;

    do {
        qsort(stats,256, sizeof(compression), &compare);
        created=false;

        for(i=0; i<255;++i) {
            if(stats[i].occur>0) {
                created=true;
                if(stats[i].tree==NULL) {
                    /* Create a new leaf */
                    np0=(node*)malloc(sizeof(node));
                    np0->nodeindex=num_nodes++;
                    np0->son0idx=255;
                    np0->son1idx=255;
                    np0->son0=NULL;
                    np0->son1=NULL;
                    np0->c=stats[i].c;
                } else {
                    /* Not a leaf */
                    np0=stats[i].tree;
                }

                if(stats[i+1].tree==NULL) {
                    np1=(node*)malloc(sizeof(node));
                    np1->nodeindex=num_nodes++;
                    np1->son0idx=255;
                    np1->son1idx=255;
                    np1->son0=NULL;
                    np1->son1=NULL;
                    np1->c=stats[i+1].c;
                } else {
                    np1=stats[i+1].tree;
                }

                np=(node*)malloc(sizeof(node));
                np->nodeindex=num_nodes++;
                np->son0=np0;
                if(np0!=NULL)
                    np->son0idx=np0->nodeindex;
                else
                    np->son0idx=-1;
                if(np1!=NULL)
                    np->son1idx=np1->nodeindex;
                else
                    np->son1idx=-1;
                np->son1=np1;
                np->c=-1;
                stats[i+1].tree=np;
                stats[i+1].occur+=stats[i].occur;
                stats[i].occur=0;
                break;
            }
        }
    } while (created);
    if (num_nodes>254) {
        printf("Compression error: more than 254 nodes in the Huffman tree.\n"
            "Code is fragile and life is short: avoid compression here...\n");
        exit(1);
    }

    return np;
}

/*  Start a new analysis. */
void init_analysis(void)
{
    unsigned int i;
    for(i=0; i<256;++i) {
        stats[i].c=(unsigned char)i;
        stats[i].tree=NULL;
        stats[i].occur=0;
    }
}

/*  Write in the file everything it is needed for decoding compressed messages
    and other descriptions. */
void output_decoder(FILE *fout)
{
    int i;
    node *np;
    fprintf(fout,"char *compressed;\n");
    fprintf(fout,"unsigned char bpointer;\n");
    fprintf(fout,"unsigned int cpointer;\n");
    fprintf(fout,"char decompress_b[B_SIZE+1];\n");
    fprintf(fout,"unsigned char currbyte;\n\n");

    np=create_tree();
    stackp=0;
    explore_tree(np);
    printf("Analyzed: %ld bytes\n",numofchar);
    fprintf(fout, "#define NUM_NODES %d\n", num_nodes);
    fprintf(fout, "const tree huftree[NUM_NODES]={\n");
    for(i=0;i<num_nodes;++i) {
        if(treearray[i].c!=-1)
            fprintf(fout, "    {%d,%d,%d},\n",treearray[i].c,
                treearray[i].son0idx, treearray[i].son1idx);
        else
            fprintf(fout, "    {255,%d,%d},\n",
                treearray[i].son0idx, treearray[i].son1idx);
    }

    fprintf(fout,"};\n\n");

    fprintf(fout,"#ifndef INTERNAL_DEF\n");
    fprintf(fout,"EFFSHORTINDEX iii;\n");
    fprintf(fout,"#endif\n");
    fprintf(fout,"char hufget(void)\n");
    fprintf(fout,"{\n");
    fprintf(fout,"#ifdef INTERNAL_DEF\n");
    fprintf(fout,"EFFSHORTINDEX iii;\n");
    fprintf(fout,"#endif\n");
    fprintf(fout,"    iii=NUM_NODES-1;\n");
    fprintf(fout,"    while(1) {\n");
    fprintf(fout,"        if(currbyte&0x1) {\n");
    fprintf(fout,"            iii=huftree[iii].son1idx;\n");
    fprintf(fout,"        } else {\n");
    fprintf(fout,"            iii=huftree[iii].son0idx;\n");
    fprintf(fout,"        }\n");
    fprintf(fout,"        if(++bpointer==8) {\n");
    fprintf(fout,"            bpointer=0;\n");
    fprintf(fout,"            currbyte=compressed[++cpointer];\n");
    fprintf(fout,"        } else {\n");
    fprintf(fout,"            currbyte>>=1;\n");
    fprintf(fout,"        }\n");
    fprintf(fout,"        if(huftree[iii].c!=255)\n");
    fprintf(fout,"            return huftree[iii].c;\n");
    fprintf(fout,"    }\n");
    fprintf(fout,"}\n\n");

    fprintf(fout,"boolean decode(void)\n");
    fprintf(fout,"{\n");
    fprintf(fout,"    register char c;\n");
    fprintf(fout,"    EFFSHORTINDEX k=0;\n");
    fprintf(fout,"    if(bpointer==0)\n");
    fprintf(fout,"       currbyte=compressed[cpointer];\n");
    fprintf(fout,"    do {\n");
    fprintf(fout,"        c=hufget();\n");
    fprintf(fout,"        SHIFTPETSCII;\n");
    fprintf(fout,"        decompress_b[k++]=c;\n");
    fprintf(fout,"    } while(c!='\\0'&&!(c==' '&&k>B_SIZE-15)&&k<B_SIZE);\n");
    fprintf(fout,"    if(c!='\\0') {\n");
    fprintf(fout,"        decompress_b[k]='\\0';\n");
    fprintf(fout,"        return true;\n");
    fprintf(fout,"    }\n");
    fprintf(fout,"    return false;\n");
    fprintf(fout,"}\n");
}

/* Write the compressed version of the given text on the file fout. */
int compress(FILE *fout, char *txt)
{
    unsigned char c;
    int i=0,j;
    char coded_v=0;
    int nbits=0;
    int val;
    int shift=0;
    int size=0;

    do {
        c=txt[i];
        val=code[c];
        for(j=length[c]-1;j>=0;--j) {
            coded_v|=((val>>j)&0x1)<<shift;
            ++shift;
            if(shift==8) {
                fprintf(fout,"0x%X,", (unsigned char)coded_v);
                ++size;
                shift=0;
                coded_v=0;
            }
        }
        ++i;
        if(i%40==0) // Avoid creating lines that are too long.
            fprintf(fout,"\n");
    } while(c!='\0');
    if(shift!=0) fprintf(fout,"0x%X", (unsigned char)coded_v);
    ++size;
    shift=0;
    coded_v=0;
    return size;
}