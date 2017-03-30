#define TYPE_FILE 0
#define TYPE_DIR 1

typedef struct node
{
    int type; /*0 if it is a file, 1 if it is a directory*/
    char *name;
    struct node *parent;
    struct node *child;
    struct node *next;
} Node;

typedef struct
{
    Node *root;
    Node *current;
} Fs_sim;

