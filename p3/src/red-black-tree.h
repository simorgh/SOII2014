/**
 *
 * Red-black tree header 
 *
 * See red-black-tree.h for details.
 * 
 */
#include "hash-table.h"

#define MAX_WORDCHR 75		//long. maxima per buffer de paraula
#define TYPE_RBTREE_PRIMARY_KEY char *  // treballarem amb cadenes de caracters 

/**
 *
 * This structure holds the information to be stored at each node. Change this
 * structure according to your needs.  In order to make this library work, you
 * also need to adapt the functions compLT, compEQ and freeRBData. For the
 * current implementation the "primary_key" member is used to index data within the
 * tree. 
 *
 */

typedef struct RBData_ {
	// The variable used to index the tree has to be called "primary_key".
	// The type may be any you want (float, char *, etc)

	TYPE_RBTREE_PRIMARY_KEY primary_key;     //la paraula serà la key del node

	// This is the additional information that will be stored
	// within the structure.
	int numFiles; 	//quantitat de fitxers en el que ha aparegut una paraula
	int *numTimes;	//llista amb quantitat de cops que apareix una paraula dins de cada fitxer fitxer
} RBData;

/**
 *
 * The node structure 
 *
 */

typedef enum { BLACK, RED } nodeColor;

typedef struct Node_ {
    /* For internal use of the structure. Do not change. */
    struct Node_ *left;         /* left child */
    struct Node_ *right;        /* right child */
    struct Node_ *parent;       /* parent */
    nodeColor color;            /* node color (BLACK, RED) */

    /* Data to be stored at each node */
    RBData *data;                    /* data stored in node */
} Node;

/**
 *
 * The tree structure. It just contains the root node, from
 * which we may go through all the nodes of the binary tree.
 *
 */

typedef struct RBTree_ {
  Node *root;           /* root of Red-Black tree */
  
  int numNodes;			/* nombre de nodes al arbre*/
  int sizeDb;			/* tamany de la base de dades*/
 
} RBTree;


/**
 * Function headers. Note that not all the functions of
 * red-black-tree.c have been included here.
 */
void initTree(RBTree *tree);
void insertNode(RBTree *tree, RBData *data);
RBData *findNode(RBTree *tree, TYPE_RBTREE_PRIMARY_KEY primary_key); 
void deleteTree(RBTree *tree);
void copyHashTableToTree(List *hashtable, RBTree *tree, int idFile, int numFiles);
void saveTree(RBTree *tree, char *filename);
RBTree * loadTree(char *filename);
double *getTreeStats(RBTree* tree);
void drawTreeStats(RBTree *tree);