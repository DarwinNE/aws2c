/* A simple Huffman compression tool. */

#include <stdio.h>
#include <stdlib.h>
#include "compress.h"

int pointer;
int bitv;
char *comp;

typedef enum boolean_t {true=-1, false=0} boolean;
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

FILE *fout;

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

void push(char c)
{
    codestack[stackp++]=c;
    valuestack<<=1;
    if(c=='1')
        ++valuestack;
}

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
    int c;
    treearray[n->nodeindex]=*n;
    /*fprintf(fout, "    {%d, %d,%d,%d},\n",n->c,n->nodeindex,
        n->son0idx, n->son1idx);*/

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
                printf("%8d %6.2f%%, code: %20s   [%c",frequencies[n->c],
                    (float)frequencies[n->c]/numofchar*100,
                    codestack, c);
                if (n->c=='\n')
                    printf("n");
                 if (n->c=='\0')
                    printf("0");
                printf("] i.e. 0x%X (0x%X on %d bits)\n",
                    (int)n->c,code[n->c],length[n->c]);
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
                    np0->son0idx=-1;
                    np0->son1idx=-1;
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
                    np1->son0idx=-1;
                    np1->son1idx=-1;
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
    if (num_nodes>255) {
        printf("Compression error: more than 255 nodes in the Huffman tree.\n"
            "Code is fragile and life is short: avoid compression here...\n");
        exit(1);
    }

    return np;
}

/*  Start a new analysis. */
void init_analysis(void)
{
    int i;
    for(i=0; i<256;++i) {
        stats[i].c=(unsigned char)i;
        stats[i].tree=NULL;
        stats[i].occur=0;
    }
}

/*  Write in the file everything it is needed for decoding compressed messages
    and other descriptions.
*/
void output_decoder(FILE *f)
{
    int i;
    fout=f;
    node *np;
    fprintf(fout,"char *compressed;\n");
    fprintf(fout,"int bpointer;\n");
    fprintf(fout,"int cpointer;\n");
    fprintf(fout,"int currbyte;\n\n");
    fprintf(fout,"int g_b(void)\n");
    fprintf(fout,"{\n");
    fprintf(fout,"   int t;\n");
    fprintf(fout,"   if(bpointer==0) {\n");
    fprintf(fout,"       currbyte=compressed[cpointer];\n");
    fprintf(fout,"   }\n");
    fprintf(fout,"   if(++bpointer==8) {\n");
    fprintf(fout,"       bpointer=0;\n");
    fprintf(fout,"       ++cpointer;\n");
    fprintf(fout,"   }\n");
    fprintf(fout,"   t=currbyte&0x1;\n");
    fprintf(fout,"   currbyte>>=1;\n");
    fprintf(fout,"   return t;\n");
    fprintf(fout,"}\n\n");

    np=create_tree();
    stackp=0;
    explore_tree(np);
    fprintf(fout, "#define NUM_NODES %d\n", num_nodes);
    fprintf(fout, "tree huftree[NUM_NODES]={\n");
    for(i=0;i<num_nodes;++i) {
        fprintf(fout, "    {%d,%d,%d},\n",treearray[i].c, treearray[i].son0idx,
            treearray[i].son1idx);
    }

    fprintf(fout,"};\n\n");

    fprintf(fout,"int hufget(void)\n");
    fprintf(fout,"{\n");
    fprintf(fout,"    int i=NUM_NODES-1;\n");
    fprintf(fout,"    while(1) {\n");
    fprintf(fout,"        if(huftree[i].c!=-1)\n");
    fprintf(fout,"            return huftree[i].c;\n");
    fprintf(fout,"        if(g_b()==0) {\n");
    fprintf(fout,"            i=huftree[i].son0idx;\n");
    fprintf(fout,"        } else {\n");
    fprintf(fout,"            i=huftree[i].son1idx;\n");
    fprintf(fout,"        }\n");
    fprintf(fout,"    }\n");
    fprintf(fout,"    return 0;\n}\n\n");
    fprintf(fout, "int decode(char *source, char* dest, int maxlen)\n");
    fprintf(fout,"{\n");
    fprintf(fout,"    char c;\n");
    fprintf(fout,"    int k=0;\n");
    fprintf(fout,"    cpointer=0;\n");
    fprintf(fout,"    bpointer=0;\n");
    fprintf(fout,"    compressed=source;\n");
    fprintf(fout,"    do {\n");
    fprintf(fout,"        c=hufget();\n");
    fprintf(fout,"        dest[k++]=c;\n");
    fprintf(fout,"    } while(c!='\\0'&&k<maxlen);\n");
    fprintf(fout,"    if(k>=maxlen)\n");
    fprintf(fout,"        return -1;\n");
    fprintf(fout,"    return 0;\n");
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
                fprintf(fout,"\\x%X", (unsigned char)coded_v);
                ++size;
                shift=0;
                coded_v=0;
            }
        }
        ++i;
    } while(c!='\0');
    fprintf(fout,"\\x%X", (unsigned char)coded_v);
    ++size;
    shift=0;
    coded_v=0;
    return size;
}