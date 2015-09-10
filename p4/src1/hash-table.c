/**
 *
 * HashTable implementation. 
 * 
 * This is an implementation of a hashtable. A minimal
 * set of necessary functions have been included.
 *
 * Igor Dzinka / Vicent Roig, 2014.
 *
 */

#include <stdio.h>
#include <stdlib.h>


/**
 * We include the hash-table.h header. Note the double
 * quotes.
 */
#include "hash-table.h"


/**
* 
* Funcio que conta el numero de elements a la hashtable local
* 
*/
int countHashtableElems(List *hashtable){
	int i;
	int numItems = 0;

	for(i = 0; i < HASHSIZE; i++) {
		numItems += hashtable[i].numItems;
		printf("hashtable[%d]numItems:\t%d\n", i, hashtable[i].numItems);
	}
	return numItems;
}

/**
 *
 * Deletes the hash table
 *
 */
void freeHashTable(List *hashTable, int size){
	int i;
	for(i = 0; i < size; i++){
		//dumpList(&(hashTable[i]));	//En finalizar el processament local imprimim el nombre de vegades que apareix cada paraula al text.
		deleteList(&(hashTable[i]));
	}
	free(hashTable);
}


/**
 *
 * Allocates memory for the hashTable. The hash table is made up of a vector
 * of linked lists. Each list is initialized with zero elements.
 *
 */
List *allocHashTable(int size){
	int i;
	List *hashTable;

	hashTable = malloc(sizeof(List) * size);
	for(i = 0; i < size; i++) initList(&(hashTable[i]));

	return hashTable;
}


/**
 *
 * This function returns the hash value for a given string
 * Function from file hash.c
 */
int getHashValue(char *cadena){
	unsigned int i, len, seed, sum, hash;

	len = strlen(cadena); 
	sum = 0;
	seed = 131;
	for(i = 0; i < len; i++) sum = sum * seed + (int)cadena[i];

	hash = sum % HASHSIZE;
	return hash;
}
