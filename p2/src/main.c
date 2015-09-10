/* * * * * * * * * * * * * * * * * * * * *
 *			[SO2] - PRACTICA 2			 *
 * +-----------------------------------+ *
 *	authors: Igor Dzinka / Vicent Roig	 *
 * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>  // per les funcions isalpha, isdigit, ...
#include "red-black-tree.h"

#define MAX_LINECHR 200		//long. maxima per buffer de linia
#define MAXCHAR 100			//long. maxima per el path del fitxer
#define MAX_WORDCHR 75		//long. maxima per buffer de paraula

#define ERR_MESSAGE__NO_MEM "Memoria insuficient!"
#define ERR_MESSAGE__FILE "Ha succeit un problema al obrir obrir el fitxer!"

typedef enum { false, true } bool;

//prototips
void processDatabase(char *dbfile, RBTree *tree);
static List *processFile(char *filename);
List * findWords(char *line, List *hashTable);


/**
 *
 *  Main function. Reads the name of database file and processes the database itself
 *
 */
int main(int argc, char **argv){
	RBTree *tree;
	tree = NULL;

	if(argc != 2) {
		printf("Utilitzacio: %s <nom configfile.cfg>\n", argv[0]);
		exit(1);
	}

	
	tree = malloc(sizeof(RBTree));
	processDatabase(argv[1], tree); //Processant base de dades...
	//printf("Paraules diferents: %d\n", tree->numNodes);

	deleteTree(tree);
	free(tree);
	return 0;
}


/**
 * 
 * Funcio que llegeix la base de dades,
 * s'encarrega del tractament dels fitxers que conte i
 * completa l'arbre RBTree
 * 
 **/ 
void processDatabase(char *configFile, RBTree * tree){
	FILE *fp;
	int i, nfiles;
	char *pname, line[MAX_LINECHR], path[MAXCHAR], file[MAXCHAR];
	pname = NULL;

	List *hashTable;	//la taula hash

	//Obrim el fitxer de base de dades
	fp = fopen(configFile, "r");
	if (!fp) {
		printf("No s'ha pogut obrir el fitxer '%s'\n", file);
		exit(2); 
	}

	fgets(line, MAXCHAR, fp); //llegim la primera linia amb el nombre de arxius a processar
	nfiles = atoi(line);	//fent servir funcio atoi com indica el manual de la practica
	if (nfiles < 1) {
		printf("El nombre d'arxius al fitxer '%s' no es correcte.\n", configFile);
		fclose(fp);
		exit(3);
	}

	/* Extract pathname: aixo sera util per poder referenciar .cfg amb paths relatius */
	pname = strrchr(configFile, '/'); 	//trobem la ultima aparicio del caracter '/' al nom de la base de dades
	if (pname == NULL)					//si no hi ha cap '/' llavors inicialitzem un string buit
		path[0] = '\0';
	else {
		strcpy(path, configFile);		//copiem la ruta de la base de dades
		path[pname-configFile+1] = '\0';//treiem a la ruta el nom de la base de dades per quedar-nos nomes amb la ruta a la carpeta on es troba
	}

	/* Init tree */
	initTree(tree);
	tree->sizeDb = nfiles;
	
	for(i=0; i < nfiles; i++) {
		// LLegim la linia 
		fgets(line, MAX_LINECHR, fp);
		line[strlen(line)-1] = '\0';	//fgets inclou el \n, per tant el substituim per el EOS.

		// Generem la ruta del fitxer a llegir
		strcpy(file, path);		//copiem la ruta fin a la carpeta (calculada abans)
		strcat(file, line);		//concatenem el nom  del fitxer i  aixi obtenim la ruta absoluta

		// Process file
		hashTable = processFile(file);	// processament del fitxer i assignacio de resultats a estructura local
		
		if (hashTable) { 				// si s'ha pogut crear l'estructura local, copiem el seu contingut a l'estructura global
			copyHashTableToTree(hashTable, tree, i, nfiles);	//copiant el contingut al arbre
			//printf("paraules desades al arbre global: %d ", tree->numNodes);
			freeHashTable(hashTable, HASHSIZE);
		}
	}

	fclose(fp);
	//printf("Nombre de nodes: %d \n", tree->numNodes);
}

/**
 *
 * Donat un fitxer extreu d'ell totes les paraules i les guarda a una hashTable
 * Retorna la hashTable amb les paraules
 *
 */
List *processFile(char *filename){
	// printf("Entrant a funcio processFile per tractar el fitxer %s.\n", filename);
	List *hashTable;
	FILE *fp = fopen(filename, "r");
	if (!fp) {
		printf("No s'ha pogut obrir el fitxer '%s'\n", filename);
		return NULL;
	}
	
	char *line = malloc(sizeof(char)*MAX_LINECHR);
	if(line == NULL){
		printf("%s", ERR_MESSAGE__NO_MEM);
		return NULL;
	}

	hashTable = allocHashTable(HASHSIZE);
	// extreiem mitjançant la funcio findWords totes les paraules del fitxer linia a linia
	while( fgets(line, MAX_LINECHR, fp)!=NULL ) hashTable = findWords(line, hashTable);

	fclose(fp);
	free(line);
	return hashTable;
}


/**
 * Donada qualsevol cadena de caràcters rebuda per referència, cerca
 * paraules seguint els criteris especificats i les guarda a una hashTable
 * */
List * findWords(char *line, List *hashTable){
	//printf("Entrant a findWords per tractar %s.\n",line);
	char c;
	char *word, *word_copy;
	int i, j, k, valor_hash;
	ListData *listData;
	
	word = malloc( sizeof(char) * MAX_WORDCHR ); /* buffer per construir les paraules */
	if(word == NULL){
		printf(ERR_MESSAGE__NO_MEM);
		exit(5);
		//return hashTable;
	}

	bool validate = true;
	c = i = j = 0;
	while ( i < MAX_LINECHR && c!='\n' ){
		c = line[i];

		if( isspace(c) || (ispunct(c) && c!='\'') ) { /* determina el final de paraula */
			// comprovem que no es tracta de una paraula buida -> tenim 1 o més caràcters al buffer
			if(j > 0){
				word[j]='\0'; /* al construir manualment la paraula, és important no oblidar introduir el final de cadena */
				
				//paraula valida; la copiem a la estructura local
				if(validate){
					word_copy = malloc(sizeof(char) * (strlen(word)+1));
					if(word_copy == NULL){
						printf(ERR_MESSAGE__NO_MEM);
						exit(4);
					}
					
					
					for(k = 0; k < strlen(word)+1; k++)	word_copy[k] = word[k]; //copiem la paraula al auxiliar word_copy
					
					valor_hash = getHashValue(word_copy);						//obtenim el seu valor hash
					listData = findList(&(hashTable[valor_hash]), word_copy);	//mirem si la paraula ja esta a la llista
					if (listData != NULL) {
						// si la trobem incrementem el numero de cops de aparicio
						listData->numTimes++;
						free(word_copy);	//en cas de no enllaçar el buffer auxiliar a listData el tenim que alliberar ara.
					} else {
						// si la paraula no esta, creem un nou node amb paraula com a clau i numTimes a 1.
						listData = malloc(sizeof(ListData));
						listData->primary_key = word_copy;
						listData->numTimes = 1;
						insertList(&(hashTable[valor_hash]), listData); //L'inserim a la llista
					}

				} //fi if validate
				
				validate = true;
				j = 0;	//reset buffer
			}
			
		} else { // el caracter no es tracta de un final de paraula.
			if(iscntrl(c) || !isascii(c) || isdigit(c)) validate = false;
			else {

				if(isupper(c)) c = tolower(c);
				
				// abans d'afegir cada un dels caracters contralem la mida del buffer
				if(j<MAX_WORDCHR){
					word[j] = c;	//caracter afegit al buffer
					j++;
				} else {
					j = 0;
				}
			}
		}
		
		i++;
	}

	free(word);
	return hashTable;
}
