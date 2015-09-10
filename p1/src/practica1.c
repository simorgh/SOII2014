/* * * * * * * * * * * * * * * * * * * * *
 *			[SO2] - PRACTICA 1		 	 *
 * +-----------------------------------+ *
 *	authors: Igor Dzinka / Vicent Roig	 *
 * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h> // per la funcio strlen
#include <ctype.h>  // per les funcions isalpha, isdigit, ...

#define MAX_LINECHR 100
#define MAX_WORDCHR	50

#define ERR_MESSAGE__NO_MEM "Memoria insuficient!"
#define ERR_MESSAGE__FILE "Ha succeit un problema al obrir obrir el fitxer!"

typedef enum { false, true } bool;

/** 
 * prototips
 * */
void findWords(char *line);



/**
 * funció principal
 * */
int main(int argc, char *argv[]){
	FILE *fp;
	char *line = malloc(sizeof(char)*MAX_LINECHR);	
	if(line == NULL){
		printf("%s", ERR_MESSAGE__NO_MEM);
		exit(1);
	}
	
	if(argc != 2) {
		printf("Utilitzacio: %s <nom fitxer>\n", argv[0]);
		exit(2);
	}

	fp = fopen(argv[1], "r");
	if(!fp){
		printf("%s", ERR_MESSAGE__FILE);
		exit(3);
	}

	while( fgets(line, MAX_LINECHR, fp)!=NULL ) findWords(line);
	
	fclose(fp);
	free(line);
	return 0;
}
/**
 * Donada qualsevol cadena de caràcters rebuda per referència, cerca
 * paraules seguint els criteris especificats i les motra per pantalla.
 * */
void findWords(char *line){
	char c;
	int i, j;
	
	char *word = malloc(sizeof(char) * MAX_WORDCHR); /* buffer per construir les paraules */
	if(word == NULL){
		printf(ERR_MESSAGE__NO_MEM);
		exit(2);
	}

	bool validate = true;
	i = j = 0;
	do {
		c = line[i];
		//printf("Caracter: %c (byte %d)\tLINE_CURSOR: %d\tWORD_INDEX: %d\n", c, c, i, j);
		
		if( isspace(c) || (ispunct(c) && c!='\'') ) { /* determina el final de paraula */
			if(j > 0){
				word[j]='\0'; /* al construir manualment la paraula, és important no oblidar introduir el final de cadena */

				if(validate) printf("\tParaula valida: %s\n", word );
				else {
					printf("\tParaula invalida: %s\n", word );
					validate = true;
				}
				j = 0;
			}
			
		} else {
			if(isdigit(c)) validate = false;
			else if(isupper(c)) c = tolower(c);
			
			word[j] = c;
			j++;
		}
		
		i++;		
	} while ( c!='\n' );
	
	free(word);
}
