/**
 *
 * HashTable header 
 * 
 * Include this file in order to be able to call the 
 * functions available in hash-table.c. We include
 * here only those information we want to make visible
 * to other files.
 *
 * Igor Dzinka / Vicent Roig, 2014.
 *
 */

#include "linked-list.h"

/**
 * 
 * The HASHSIZE is used to define the number of entries of the
 * hash-table array.
 *
 */
#define HASHSIZE  10000	 //numero de elements de la taula hash

/**
 *
 * Function heders we want to make visible so that they
 * can be called from any other file.
 *
 */
int getHashValue(char *cadena);
int countHashtableElems(List *hashtable);
List *allocHashTable(int size);
void freeHashTable(List *hashTable, int size);
