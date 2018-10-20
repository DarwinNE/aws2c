#ifndef __COMPRESS_H__
#define __COMPRESS_H__

typedef struct node_t {
    int c;
    int nodeindex;
    int son0idx;
    int son1idx;
    struct node_t *son0;
    struct node_t *son1;
} node;

typedef struct compression_t {
    unsigned char c;
    node *tree;
    long occur;
} compression;

void analyze(char *text);
int compare(const void *a1, const void *a2);
void push(char c);
char pop(void);
void indent(void);
void explore_tree(node *n);
node *create_tree(void);
void init_analysis(void);
void output_decoder(FILE *f);
int compress(FILE *fout, char *txt);
int get_max_len(void);




#endif