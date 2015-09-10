/**
 *
 * Red-black tree implementation. 
 * 
 * Binary search trees work best when they are balanced or the path length from
 * root to any leaf is within some bounds. The red-black tree algorithm is a
 * method for balancing trees. The name derives from the fact that each node is
 * colored red or black, and the color of the node is instrumental in
 * determining the balance of the tree. During insert and delete operations,
 * nodes may be rotated to maintain tree balance. Both average and worst-case
 * search time is O(lg n).
 *
 * This implementation is original from John Morris, University of Auckland, at
 * the following link
 *
 * http://www.cs.auckland.ac.nz/~jmor159/PLDS210/niemann/s_rbt.htm
 *
 * and has been adapted here by Lluis Garrido, 2014.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "red-black-tree.h"


/**
 * support functions prototypes
 */
void saveNodesRecursive(Node *node, int *sizeDb, FILE *fp);
void saveNodeData(Node *node, int *sizeDb, FILE *fp);
void getStatsRecursive(Node *node, double *treeStats);
void getTreeStatsRecursive(Node *node, double *treeStats);




/**
 *
 * Free data element. The user should adapt this function to their needs.  This
 * function is called internally by deleteNode.
 *
 */
static void freeRBData(RBData *data/*, int *sizeDb*/){
	//print de la tupla amb les dades sol·licitades abans de alliberar memòria
/*
	int i, sum = 0;
	for(i=0; i<*sizeDb; i++) sum += data->numTimes[i];
	printf("(%d,%s)\n", sum, data->primary_key);
*/
	if (data->primary_key) free(data->primary_key);
	if (data->numTimes) free(data->numTimes);
	free(data);
}

/**
 *
 * Compares if primary_key1 is less than primary_key2. Should return 1 (true) if condition
 * is satisfied, 0 (false) otherwise.
 *
 */
static int compLT(TYPE_RBTREE_PRIMARY_KEY key1, TYPE_RBTREE_PRIMARY_KEY key2){
	int rc = 0;
	if (strcmp(key1, key2) < 0) rc = 1;
	return rc;
}


/**
 *
 * Compares if primary_key1 is equal to primary_key2. Should return 1 (true) if condition
 * is satisfied, 0 (false) otherwise.
 *
 */
static int compEQ(TYPE_RBTREE_PRIMARY_KEY key1, TYPE_RBTREE_PRIMARY_KEY key2){
	int rc = 0;
	if (strcmp(key1, key2) == 0) rc = 1;
	return rc;
}

/**
 * 
 * PLEASE DO NOT CHANGE THE CODE BELOW UNLESS YOU REALLY KNOW WHAT YOU ARE
 * DOING.
 *
 */
#define NIL &sentinel           /* all leafs are sentinels */
static Node sentinel = { NIL, NIL, 0, BLACK, NULL };

/**
 * 
 * Initialize the tree.
 * 
 */
void initTree(RBTree *tree){
	tree->root = NIL;
	tree->numNodes = 0;			/* nombre de nodes al arbre*/
	tree->sizeDb = 0;				/* tamany de la base de dades*/
	
}


/**
 *
 *  Rotate node x to left. Should not be called directly by the user. This
 *  function is used internally by other functions.
 *
 */
static void rotateLeft(RBTree *tree, Node *x) {
	Node *y = x->right;

	/* establish x->right link */
	x->right = y->left;
	if (y->left != NIL) y->left->parent = x;

	/* establish y->parent link */
	if (y != NIL) y->parent = x->parent;
	if (x->parent) {
		if (x == x->parent->left) x->parent->left = y;
		else x->parent->right = y;
	} else {
		tree->root = y;
	}

	/* link x and y */
	y->left = x;
	if (x != NIL) x->parent = y;
}

/**
 *  
 *  Rotate node x to right. Should not be called directly by the user. This
 *  function is used internally by other functions.
 *
 */
static void rotateRight(RBTree *tree, Node *x) {
	Node *y = x->left;

	/* establish x->left link */
	x->left = y->right;
	if (y->right != NIL) y->right->parent = x;

	/* establish y->parent link */
	if (y != NIL) y->parent = x->parent;
	if (x->parent) {
		if (x == x->parent->right) x->parent->right = y;
		else x->parent->left = y;
	} else {
		tree->root = y;
	}

	/* link x and y */
	y->right = x;
	if (x != NIL) x->parent = y;
}

/** 
 *
 * Maintain Red-Black tree balance  after inserting node x. Should not be
 * called directly by the user. This function is used internally by other
 * functions.
 *
 */
static void insertFixup(RBTree *tree, Node *x) {
	/* check Red-Black properties */
	while (x != tree->root && x->parent->color == RED) {
		/* we have a violation */
		if (x->parent == x->parent->parent->left) {
			Node *y = x->parent->parent->right;
			
			if (y->color == RED) {
				/* uncle is RED */
				x->parent->color = BLACK;
				y->color = BLACK;
				x->parent->parent->color = RED;
				x = x->parent->parent;
			} else {
				/* uncle is BLACK */
				if (x == x->parent->right) {
					/* make x a left child */
					x = x->parent;
					rotateLeft(tree,x);
				}
				/* recolor and rotate */
				x->parent->color = BLACK;
				x->parent->parent->color = RED;
				rotateRight(tree, x->parent->parent);
			}
		} else {
			/* mirror image of above code */
			Node *y = x->parent->parent->left;
			
			if (y->color == RED) {
				/* uncle is RED */
				x->parent->color = BLACK;
				y->color = BLACK;
				x->parent->parent->color = RED;
				x = x->parent->parent;
			} else {
				/* uncle is BLACK */
				if (x == x->parent->left) {
					x = x->parent;
					rotateRight(tree, x);
				}
				x->parent->color = BLACK;
				x->parent->parent->color = RED;
				rotateLeft(tree,x->parent->parent);
			}
		}
	}
	tree->root->color = BLACK;
}

/**
 *  
 * Allocate node for data and insert in tree. This function does not perform a
 * copy of data when inserting it in the tree, it rather creates a node and
 * makes this node point to the data. Thus, the contents of data should not be
 * overwritten after calling this function.
 *
 */
void insertNode(RBTree *tree, RBData *data) {
	Node *current, *parent, *x;

	/* Find where node belongs */
	current = tree->root;
	parent = 0;
	while (current != NIL) {
		if (compEQ(data->primary_key, current->data->primary_key)) {
			printf("insertNode: trying to insert but primary key is already in tree.\n");
			exit(1);
			//return current;
		}
		parent = current;
		current = compLT(data->primary_key, current->data->primary_key) ? current->left : current->right;
	}

	/* setup new node */
	if ((x = malloc (sizeof(*x))) == 0) {
		printf ("insufficient memory (insertNode)\n");
		exit(1);
	}

	/* Note that the data is not copied. Just the pointer
	 is assigned. This means that the pointer to the 
	 data should not be overwritten after calling this
	 function. */
	x->data = data;

	/* Copy remaining data */
	x->parent = parent;
	x->left = NIL;
	x->right = NIL;
	x->color = RED;

	/* Insert node in tree */
	if(parent) {
		if(compLT(data->primary_key, parent->data->primary_key)) parent->left = x;
		else parent->right = x;
	} else {
		tree->root = x;
	}

	tree->numNodes++;
	insertFixup(tree, x);
}

/**
 *
 *  Find node containing the specified primary_key. Returns the node.
 *  Returns NULL if not found.
 *
 */
RBData * findNode(RBTree *tree, TYPE_RBTREE_PRIMARY_KEY primary_key) {

  Node *current = tree->root;
  while(current != NIL)
    if(compEQ(primary_key, current->data->primary_key))
      return (current->data);
    else
      current = compLT(primary_key, current->data->primary_key) ?
	current->left : current->right;

 return NULL;
}

/**
 *
 *  Function used to delete a tree. Do not call directly. 
 *
 */
static void deleteTreeRecursive(Node *x/*, int *sizeDb*/){
	if (x->right != NIL) deleteTreeRecursive(x->right/*, sizeDb*/);
	if (x->left != NIL) deleteTreeRecursive(x->left/*, sizeDb*/);

	freeRBData(x->data/*, sizeDb*/);
	free(x);
}


/**
 *
 *  Delete a tree. All the nodes and all the data pointed to by
 *  the tree is deleted. 
 *
 */
void deleteTree(RBTree *tree){
	//int *ptr = &(tree->sizeDb);
	if (tree->root != NIL) deleteTreeRecursive(tree->root/*, ptr*/);
}


/**
 *
 * Funció que copia el contingut de una hashtable al arbre global
 *
 */ 
void copyHashTableToTree(List *hashtable, RBTree *tree, int idFile, int numFiles){
	RBData *data;
	ListItem *current;

	char *paraula;
	int i, j, len, numItems;

	for(i = 0; i < HASHSIZE; i++) {
		numItems = hashtable[i].numItems;
		current = hashtable[i].first;

		for(j = 0; j < numItems; j++) {
			/* Search if the key is in the tree */
			data = findNode(tree, current->data->primary_key);
			
			if (data != NULL) {
				//printf("\t[RED_BLACK_TREE][paraula ja continguda!!][%s]\n", current->data->primary_key);
				
				data->numFiles++;
				data->numTimes[idFile] = current->data->numTimes;
			} else {
				// If the key is not in the tree, allocate memory for the data and insert in the tree.
				data = malloc(sizeof(RBData));
				len = strlen(current->data->primary_key);		//mirem tamany de la paraula

				paraula = malloc(sizeof(char) * (len + 1));		//reservem espai
				strcpy(paraula, current->data->primary_key);	//copiem la paraula

				data->primary_key = paraula;	//asignem la paraula com primary  key
				data->numFiles = 1;				//es un nou node per tant numFiles val 1

				data->numTimes = calloc(numFiles, sizeof(int));//reservem memoria per a array amb cops que surt la paraula a cada fitxer 
				//for(k = 0; k < numFiles; k++)  data->numTimes[k] = 0;	//omplim array amb zeros

				data->numTimes[idFile] = current->data->numTimes;	// a la posicio  del fitxer actual li posem un 1
				
				insertNode(tree, data);							//inserim el node
				//printf("\t[RED_BLACK_TREE][nova paraula][%s][numNodes%d]\n", current->data->primary_key, tree->numNodes);
			}
			
			current = current->next;	//avancem el punter dins de la llista
		}
	}
}

/**
 * Functions used to save the RBTree into the specified binary file.
 * Based on deleteTree / deleteRecursive functions.
 */

void saveTree(RBTree *tree, char* filename){
	if (tree->root != NIL){
		FILE *fp;
		fp = fopen(filename, "w");
		if(!fp) return;

		fwrite(&(tree->sizeDb), sizeof(int), 1, fp );
		fwrite(&(tree->numNodes), sizeof(int), 1, fp );
		
		saveNodesRecursive(tree->root, &(tree->sizeDb), fp);
		fclose(fp);
	}
}

void saveNodesRecursive(Node *node, int *sizeDb, FILE *fp){
	 saveNodeData(node, sizeDb, fp);
	 
	 if(node->left != NIL) saveNodesRecursive(node->left, sizeDb,fp);
	 if(node->right != NIL) saveNodesRecursive(node->right, sizeDb,fp);
}

void saveNodeData(Node *node, int *sizeDb, FILE *fp){
	
	int length = strlen(node->data->primary_key);
	fwrite(&(length), sizeof(int), 1, fp);
	fwrite(node->data->primary_key, sizeof(char), length, fp);
	fwrite(&(node->data->numFiles), sizeof(int), 1, fp);
	fwrite(node->data->numTimes, sizeof(int), *sizeDb, fp );
}


/**
 * Functions used to load the RBTree from a specified binary file.
 */

RBTree * loadTree(char *filename){
	RBTree *tree = malloc(sizeof(RBTree));
	initTree(tree);

	FILE *fp;
	fp = fopen(filename, "r");
	if(!fp){

		deleteTree(tree);
		free(tree);
		return NULL;
	}
	
	int sizeDb, numNodes = NULL;
	fread( &(sizeDb), sizeof(int), 1, fp );
	fread( &(numNodes), sizeof(int), 1, fp );
	if(numNodes == 0){
		fclose(fp);
		deleteTree(tree);
		free(tree);
		return NULL;
	}

	int i;
	for(i=0; i< numNodes; i++){
		RBData *data = malloc(sizeof(RBData));
		int length = NULL;

		fread(&(length), sizeof(int), 1, fp);
		data->primary_key = malloc(sizeof(char) * (length+1) );
		fread(data->primary_key, sizeof(char), length, fp);
		data->primary_key[length] = '\0';

		fread(&(data->numFiles), sizeof(int), 1, fp);
		data->numTimes = malloc(sizeof(int) * sizeDb);
		fread(data->numTimes, sizeof(int), sizeDb, fp);

		insertNode(tree, data);
	}

	fclose(fp);
	return tree;
}

/**
 * funció que calcula les dades per a histograma 
 */
double * getTreeStats(RBTree* tree){
	int i;
	double *treeStats = calloc(MAX_WORDCHR,sizeof(double));
	getTreeStatsRecursive(tree->root, treeStats);

	//fem la normalització  de les dades;
	for(i = 0; i < MAX_WORDCHR; i++) treeStats[i] /= tree->numNodes;

	return treeStats;
}

void getTreeStatsRecursive(Node *node, double *treeStats){
	if (node != NIL){
		int len = strlen(node->data->primary_key);
		treeStats[len-1] += 1.0;
	
		getTreeStatsRecursive(node->left, treeStats);
		getTreeStatsRecursive(node->right, treeStats);
	}
}

void drawTreeStats(RBTree *tree){
	double *treeStats = getTreeStats(tree); //obtenim les dades per a histograma
	FILE *fp,*fpout;
	
	fp = fopen("../proves/treeStats.dat", "w");
	int i;
	for(i = 1; i <= MAX_WORDCHR; i++) fprintf(fp, "%d %f\n", i, treeStats[i-1]);
	fclose(fp);
	free(treeStats);

	fpout = popen("gnuplot -persist", "w");
	if (!fpout){
		printf("ERROR: no puc crear canonada.\n");
		exit(2);
	}

	fprintf(fpout, "set term svg\n");
	fprintf(fpout, "set out \"../proves/treeHistogram.svg\"\n");
	fprintf(fpout, "plot \"../proves/treeStats.dat\" with impulses\n");
	fflush(fpout);

	//exit gnuplot
	fprintf(fpout, "exit\n");

	pclose(fpout);
}

