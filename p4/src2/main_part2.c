/* * * * * * * * * * * * * * * * * * * * *
 *			[SO2] - PRACTICA 4			 *
 * +-----------------------------------+ *
 *	authors: Igor Dzinka / Vicent Roig	 *
 * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>  		// per les funcions isalpha, isdigit, ...
#include <unistd.h>			// per la funció acces()
#include <pthread.h>
#include "red-black-tree.h"

#define MAX_LINECHR 200		// long. maxima per buffer de linia
#define MAXCHAR 100			// long. maxima per el path del fitxer
#define NTHREADS 4			// nombre de fils a executar

#define ERR_MESSAGE__NO_MEM "Memoria insuficient!"
#define ERR_MESSAGE__FILE "Ha succeit un problema al obrir obrir el fitxer!"

typedef enum { false, true } bool;

pthread_mutex_t lockFilelist;//  = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexP = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexC = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condP, condC;

//int NTHREADS;
List** buffer;
int* buffer_index; // necesario para que el consumidor sepa que indice tiene el fichero procesado...
int w = 0, r = 0;
int comptador = 0; // nombre d’elements ocupats
int processats = 0;
int th_count = 0;
int indexFile = -1; //indicador del fichero utilizado por threads

struct arg_struct_producer{
	int* nfiles;
	char** fileList;
};

struct arg_struct_consumer{
	int* nfiles;
	RBTree* tree;
};


//prototips
List* processFile(char *filename);
List* findWords(char *line, List *hashTable);
RBTree* createTree(char** fileList, int* nfiles);
char** readDatabase(char *configFile, int* nfiles);
void processDatabase(char** fileList, RBTree * tree, int *nfiles,  int* tid);
void* thread_fn(void *arg);
void* thread_p(void* arg);
void* thread_c(void* arg);


int menu(){
	char opt;
	
	system ("clear");
	printf("╔═══════════════════════════╗\n");
	printf("║     Menu de l'aplicació   ║\n");
	printf("╠═══════════════════════════╣\n");
	printf("║  1. Crear arbre           ║\n");
	printf("║  2. Emmagatzemar arbre    ║\n");
	printf("║  3. Carregar arbre        ║\n");
	printf("║  4. Histograma de l'arbre ║\n");
	printf("║  5. Sortir                ║\n");
	printf("╚═══════════════════════════╝\n");
	
	printf("► Introdueix opció: ");
	fflush(stdin);
	scanf("%c", &opt);
	return opt;
}


/**
 *
 *  Main function. Reads the name of database file and processes the database itself
 *
 */
int main(int argc, char **argv){
	char opcio;
	RBTree *tree =  NULL;
	char *filename = malloc(sizeof(char)*MAXCHAR);
	char** fileList = NULL;
	int nfiles, i;
	

	//NTHREADS = sysconf(_SC_THREAD_THREADS_MAX) * 2;//sysconf(_SC_NPROCESSORS_CONF);//sysconf(_SC_NPROCESSORS_ONLN);
	do {
		opcio = menu();
		switch(opcio){

			case '1' :	//crear arbre
				printf("► Fitxer de carrega: ");
				scanf("%s", filename);

				if( access(filename, F_OK )!=-1 ) { // file exists
					if(tree){	//in case there is alreadey a tree
						deleteTree(tree);
						free(tree);
					}
					
					//llegim la base de dades i guardem el contingut a fileList
					fileList = readDatabase(filename, &nfiles);
					indexFile = -1;
					processats = 0;
					comptador = 0;
					th_count = 0;
					tree = createTree(fileList, &nfiles);

					printf("\nParaules diferents: %d", tree->numNodes);
					fgetc(stdin);

				} else {
					printf("▬ El fitxer de carrega especificat no es valid.\n");
					fgetc(stdin);
				}

				fflush(stdin);
				fgetc(stdin);

				break;

			case '2' : //emmagatzemar arbre
				if(tree){
					printf("► Nom del fitxer: ");
					scanf("%s", filename);
					saveTree(tree, filename);
				} else {
					fflush(stdin);
					printf("▬ No hi ha cap arbre per emmagatzemar\n");
					fgetc(stdin);
				}
				fgetc(stdin);
				break;

			case '3' : //Carregar arbre
				printf("► Fitxer de l'arbre: ");
				scanf("%s", filename);
				if( access(filename, F_OK )!=-1 ) { // file exists
					if(tree){	//in case there is alreadey a tree
						deleteTree(tree);
						free(tree);
					}
					tree = loadTree(filename);

					if(tree) printf("▬ Arbre Carregat. Paraules diferents: %d", tree->numNodes);
					else  printf("▬ Error al carregar l'arbre");

				} else { // file does not exist
					fflush(stdin);
					printf("▬ El fitxer especificat no existeix.\n");
				}
				fgetc(stdin);
				fgetc(stdin);
				break;

			case '4' :	//Mostrar histogrames
				//printf("► Opcio mostrar histogrames encara per implementar\n");
				if(tree){
					drawTreeStats(tree);
					fflush(stdin);
					printf("▬ Grafica 'treeHistogram.svg' generada.");
				}else{
					fflush(stdin);
					printf("▬ Error. No s'ha trobat arbre carregat.");
				}
				fgetc(stdin);
				fgetc(stdin);
				break;

			case '5' :	//Sortir
				printf("Exit\n");
				break;

			default:
				fflush(stdin);
				printf("\n▬ Opció introduida no es valida. (pulsa tecla per continuar)\n");
				fgetc(stdin);
		}
	} while(opcio!= '5');	


	if(tree){
		deleteTree(tree);
		free(tree);
	}
	if(filename) free(filename);
	if(fileList){
		for(i = 0;i< nfiles;i++) free(fileList[i]);
		free(fileList);
	}

	return 0;
}


RBTree* createTree(char** fileList, int* nfiles){
	int i, err;
	pthread_t tid[NTHREADS+1];

    /* Buffering allocation */
	if ((buffer = (List**) malloc((NTHREADS)*sizeof( List*))) == NULL) return NULL; // reserva de memoria per el buffer
	if ((buffer_index = (int*) malloc((NTHREADS)*sizeof( int))) == NULL) return NULL; // reserva de memoria per el buffer dels index

	RBTree *tree = malloc(sizeof(RBTree));
    /* Init tree */
	initTree(tree);
	tree->sizeDb = *nfiles;

    struct  arg_struct_producer args_p;
    	args_p.nfiles = nfiles;
    	args_p.fileList = fileList;
	
	struct  arg_struct_consumer args_c;
    	args_c.nfiles = nfiles;
		args_c.tree = tree;

    if (pthread_mutex_init(&lockFilelist, NULL) != 0){
	    printf("\n lockFilelist init failed\n");
	    return NULL;
    }

    pthread_cond_init(&condP, NULL);
    pthread_cond_init(&condC, NULL);

    //creació de thread consumidor
    if( (err = pthread_create(&tid[0], NULL, &thread_c, (void *) &args_c)) != 0){
    		printf("\ncan't create thread :[%s]", strerror(err));
    		return NULL;
    }

	//creaació dels threads productor
    for(i=1; i <= NTHREADS; i++){
    	if( (err = pthread_create(&tid[i], NULL, &thread_p, (void *) &args_p)) != 0){
    		printf("\ncan't create thread :[%s]", strerror(err));
    		return NULL;
    	}
    }

    /* El fil principal es quedarà esperant que els fils creats finalitzin la creacio de l’arbre */
    for(i=0; i <= NTHREADS; i++){
    	pthread_join(tid[i], NULL);
    }
    

    pthread_mutex_destroy(&lockFilelist);
	free(buffer);
	free(buffer_index);

    return args_c.tree;
}


/**
 * Funció per llegir el fitxer de configuració i guardar el seu contingut a una llista que es passa per referencia
 */
char** readDatabase(char *configFile, int* nfiles){
	FILE *fp;
	int i;
	char *pname;
	char** fileList = NULL;
	char line[MAX_LINECHR], path[MAXCHAR], file[MAXCHAR];
	pname = NULL;
	
	fp = fopen(configFile, "r");
	if (!fp) {
		printf("No s'ha pogut obrir el fitxer '%s'\n", configFile);
		return NULL;
	}

	fgets(line, MAXCHAR, fp);	//llegim la primera linia amb el nombre de arxius a processar
	*nfiles = atoi(line);		//fent servir funcio atoi com indica el manual de la practica
	printf("nfiles = %d\n",*nfiles);
	if ( *nfiles < 1) {
		printf("El nombre d'arxius al fitxer '%s' no es correcte.\n", configFile);
		fclose(fp);
		return NULL;
	}

	//reservem memoria per a la llista de fitxers
	if ((fileList = (char**) malloc((*nfiles)*sizeof( char*))) == NULL) return NULL; 
	

	/* Extract pathname: aixo sera util per poder referenciar .cfg amb paths relatius */
	pname = strrchr(configFile, '/'); 	//trobem la ultima aparicio del caracter '/' al nom de la base de dades
	if (pname == NULL)					//si no hi ha cap '/' llavors inicialitzem un string buit
		path[0] = '\0';
	else {
		strcpy(path, configFile);		//copiem la ruta de la base de dades
		path[pname-configFile+1] = '\0';//treiem a la ruta el nom de la base de dades per quedar-nos nomes amb la ruta a la carpeta on es troba
	}
	
	for(i=0; i < (*nfiles); i++) {
		// LLegim la linia 
		fgets(line, MAX_LINECHR, fp);
		line[strlen(line)-1] = '\0';	//fgets inclou el \n, per tant el substituim per el EOS.

		// Generem la ruta del fitxer a llegir
		strcpy(file, path);		//copiem la ruta fin a la carpeta (calculada abans)
		strcat(file, line);		//concatenem el nom  del fitxer i  aixi obtenim la ruta absoluta
		
		//reservem tant espai com es requereix per guardar la ruta
		if ((fileList[i] = malloc( sizeof(char)*(strlen(file)+1) )) == NULL) return NULL;

		//guardem  la ruta a la llista de fitxers
		strcpy(fileList[i], file);
		//printf("IT%d -- STRCPY RESULT [%s]->[%s]\n", i, file, fileList[i]);
	}
	
	fclose(fp);	//tanquem el fitxer de base de dades
	return fileList;
}


/**
 *
 * Donat un fitxer extreu d'ell totes les paraules i les guarda a una hashTable
 * Retorna la hashTable amb les paraules
 *
 */
List* processFile(char* filename){
	
	List *hashTable;
	FILE *fp;

	fp = fopen(filename, "r");
	if (!fp) {
		printf("\nNo s'ha pogut obrir el fitxer '%s'", filename);
		return NULL ;
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



/* * * * * * * * * * * * * * * * * *
 *   
 *	Consumer/Producer functions
 *
 * * * * * * * * * * * * * * * * * */

void consume(RBTree * tree, int *nfiles ){
	
	List* hashTable = buffer[r];
	int index = buffer_index[r];
	
	if (hashTable) { 				// si s'ha pogut crear l'estructura local, copiem el seu contingut a l'estructura global
		//printf("\n\t\t[thread] > SIZE_T: %d [BEFORE] del fitxer %d", tree->numNodes, index);
		copyHashTableToTree(hashTable, tree, index, nfiles);	//copiant el contingut al arbre
		printf("\n\t\t[thread] > SIZE_T: %d [AFTER] del fitxer %d", tree->numNodes, index);

		//printf("paraules desades al arbre global: %d ", tree->numNodes);
		freeHashTable(hashTable, HASHSIZE);
	}
}


void* thread_p(void* arg){
	struct arg_struct_producer *args = (struct arg_struct_producer *) arg;

	char* filename;
	int localIndex;
	List* hashTable;	//la taula hash
	
	//bloqueo del acceso a la lista de ficheros
	pthread_mutex_lock(&lockFilelist);
	indexFile++;
	localIndex = indexFile;
	pthread_mutex_unlock(&lockFilelist);
	
	
	while(localIndex < *args->nfiles){
		
		if(localIndex >= *args->nfiles){
			buffer[w] = NULL;		//copia la estructura local al buffer
			buffer_index[w] = localIndex;
			return NULL;
		}

		// Process file
		filename = args->fileList[localIndex];
		printf("\n\t[thread ] > Entrant a processFile per tractar el fitxer %s", filename);
		hashTable = processFile(filename);	// processament del fitxer i assignacio de resultats a estructura local
		
		

		pthread_mutex_lock(&mutexP);
		while (comptador == NTHREADS) {
			pthread_cond_wait(&condP, &mutexP);
		}

		buffer[w] = hashTable;		//copia la estructura local al buffer
		buffer_index[w] = localIndex;	
		
		w = (w+1)%NTHREADS;
		comptador++;
		
		//bloqueo del acceso a la lista de ficheros
		pthread_mutex_lock(&lockFilelist);
		indexFile++;
		localIndex = indexFile;
		pthread_mutex_unlock(&lockFilelist);

		pthread_cond_signal(&condC);
		pthread_mutex_unlock(&mutexP);
	}
	return NULL;
}


void* thread_c(void* arg){
	struct arg_struct_consumer *args = (struct arg_struct_consumer *) arg;
	while(processats != *args->nfiles){

		pthread_mutex_lock(&mutexC);
		while (comptador == 0) {
			//printf("\n**** CONSUMIDOR Esperant...");
			pthread_cond_wait(&condC, &mutexC);
		}

		//printf("\n\t-> Consuming...");
		consume( args->tree, args->nfiles);
		r = (r+1)%NTHREADS;
		comptador--;
		processats++;

		pthread_cond_broadcast(&condP);
		pthread_mutex_unlock(&mutexC);
		
	}
	return NULL;
}