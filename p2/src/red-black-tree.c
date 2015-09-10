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
 *
 * Free data element. The user should adapt this function to their needs.  This
 * function is called internally by deleteNode.
 *
 */
static void freeRBData(RBData *data, int *sizeDb){
	//print de la tupla amb les dades sol·licitades abans de alliberar memòria
	int i, sum = 0;
	for(i=0; i<*sizeDb; i++) sum += data->numTimes[i];

	printf("(%d,%s)\n", sum, data->primary_key);
	
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
Node *findNode(RBTree *tree, TYPE_RBTREE_PRIMARY_KEY primary_key) {

  Node *current = tree->root;
  while(current != NIL)
    if(compEQ(primary_key, current->data->primary_key))
      return (current);
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
static void deleteTreeRecursive(Node *x, int *sizeDb){
	if (x->right != NIL) deleteTreeRecursive(x->right, sizeDb);
	if (x->left != NIL) deleteTreeRecursive(x->left, sizeDb);

	freeRBData(x->data, sizeDb);
	free(x);
}


/**
 *
 *  Delete a tree. All the nodes and all the data pointed to by
 *  the tree is deleted. 
 *
 */
void deleteTree(RBTree *tree){
	int *ptr = &(tree->sizeDb);
	if (tree->root != NIL) deleteTreeRecursive(tree->root, ptr);
}


/**
 *
 * Funció que copia el contingut de una hashtable al arbre global
 *
 */ 
void copyHashTableToTree(List *hashtable, RBTree *tree, int idFile, int numFiles){
	Node *t;
	RBData *data;
	ListItem *current;

	char *paraula;
	int i, j, len, numItems;

	for(i = 0; i < HASHSIZE; i++) {
		numItems = hashtable[i].numItems;
		current = hashtable[i].first;

		for(j = 0; j < numItems; j++) {
			/* Search if the key is in the tree */
			t = findNode(tree, current->data->primary_key);
			
			if (t != NULL) {
				//printf("\t[RED_BLACK_TREE][paraula ja continguda!!][%s]\n", current->data->primary_key);
				
				t->data->numFiles++;
				t->data->numTimes[idFile] = current->data->numTimes;
			} else {
				// If the key is not in the tree, allocate memory for the data and insert in the tree.
				data = malloc(sizeof(RBData));
				len = strlen(current->data->primary_key);	//mirem tamany de la paraula

				paraula = malloc(sizeof(char) * (len + 1));	//reservem espai
				strcpy(paraula, current->data->primary_key);	//copiem la paraula

				data->primary_key = paraula;			//asignem la paraula com primary  key
				data->numFiles = 1;				//es un nou node per tant numFiles val 1

				data->numTimes = calloc(numFiles, sizeof(int));//reservem memoria per a array amb cops que surt la paraula a cada fitxer 
				//for(k = 0; k < numFiles; k++)  data->numTimes[k] = 0;	//omplim array amb zeros

				data->numTimes[idFile] = current->data->numTimes;	// a la posicio  del fitxer actual li posem un 1
				
				insertNode(tree, data);					//inserim el node
				//printf("\t[RED_BLACK_TREE][nova paraula][%s][numNodes%d]\n", current->data->primary_key, tree->numNodes);
			}
			
			current = current->next;	//avancem el punter dins de la llista
		}
	}
}
