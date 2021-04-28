/*
   Tue 27 Apr 2021 06:01:00 PM CEST
   Developed by Maximilian Strele

   Description
   Passwordmanager using it's own encryption algorithm
   TODO better password and keygenerator
   TODO password remove
*/

#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>

#include "config.h"
#include "encryption.h"

/* Function declarations */
static void getPassWord(char **, int *, FILE *);
static void passWordGen(struct messageArray *);
static void keyGen(struct keyArray *);

/* Function implementations */
void
getPassWord(char **lineptr, int *n, FILE *stream)
{
	/* Variables */
	struct termios old, new;

	/* Logic */
	tcgetattr(fileno(stream), &old);
	new = old;
	new.c_lflag &= ~ECHO;
	tcsetattr(fileno(stream), TCSAFLUSH, &new);
	*n = getline(lineptr, n, stream);
	(void)tcsetattr(fileno(stream), TCSAFLUSH, &old);
}

void
passWordGen(struct messageArray *password)
{
	/* Variables */
	char seed;

	/* Logic */
	srand(time(0));
	for (unsigned int i=0;i<password->length-1;i++) {
		seed = getchar();
		if (seed=='\n') {
			srand(time(0));
			continue;
		}
		for (uint8_t a=0;a>=seed;a++) {
			rand();
		}
		password->array[i] = 0x21 + rand() % (0x7f - 0x21);
	}
	password->array[password->length-1] = '\0';
}

void
keyGen(struct keyArray *key)
{
	/* Variables */
	char seed;

	/* Logic */
	srand(time(0));
	for (uint16_t i=0;i<key->length;i++) {
		seed = getchar();
		if (seed=='\n') {
			srand(time(0));
			continue;
		}
		for (uint8_t a=0;a>=seed;a++) {
			rand();
		}
		key->array[i] = rand() % 0xff;
	}
}

int
main(int argc, char *argv[])
{
	/* Variables */
	struct keyArray *key;
	struct messageArray *password;
	FILE *file;
	FILE *keyFile;
	char *path;
	char *temp;
	int pathLen;
	uint16_t tempLen;

	/* Logic */
	if (argc<3) {
		printf("Too few arguments!");
		exit(0);
	}
	key = malloc(sizeof(*key));

	if (strcmp(argv[1],"-keygen")!=0) {
		temp = NULL;
		path = NULL;
		password = malloc(sizeof(*password));
		pathLen = strlen(getenv("HOME"));
		pathLen += strlen("/.pmanager/");
		pathLen += strlen(argv[argc-1]);
		path = malloc(pathLen*sizeof(*path));
		strcat(path, getenv("HOME"));
		strcat(path, "/.pmanager/");
		strcat(path, argv[argc-1]);

		keyFile = fopen(keyPath, "r");
		if (keyFile==NULL) {
			fprintf(stderr, "No keyfile!\nexit code: %d\n", errno);
			exit(EXIT_FAILURE);
		}
		temp = malloc(sizeof(*temp)*sizeof(key->length));
		tempLen = getline(&temp, &tempLen, keyFile);
		key->length = strtol(temp, NULL, 16);
		key->array = malloc(key->length*sizeof(key->array[0]));
		for (uint16_t i=0;i<key->length;i++) {
			tempLen = getline(&temp, &tempLen, keyFile);
			key->array[i] = strtol(temp, NULL, 16);
		}
		free(temp);
		fclose(keyFile);
	}

	if (strcmp(argv[1],"-set")==0) {
		printf("password ('str'): ");
		getPassWord(&password->array, &password->length, stdin);
		cbcEnc(&password, key);
		file = fopen(path, "w");
		if (file==NULL) {
			fprintf(stderr, "Permission denied!\nexit code: %d\n", errno);
			exit(EXIT_FAILURE);
		}
		fputs(password->array, file);
	} else if (strcmp(argv[1],"-gen")==0) {
		if (argc<4) {
			printf("Too few arguments at program start!");
			exit(0);
		}
		password->length = strtol(argv[2], NULL, 10)+1;
		password->array = malloc(
				password->length*sizeof(password->array[0]));
		printf("seed (length=passwordlength): ");
		passWordGen(password);
		cbcEnc(&password, key);
		file = fopen(path, "w");
		if (file==NULL) {
			fprintf(stderr, "Permission denied!\nexit code: %d\n", errno);
			exit(EXIT_FAILURE);
		}
		fputs(password->array, file);
	} else if (strcmp(argv[1],"-get")==0) {
		file = fopen(path, "r");
		if (file==NULL) {
			fprintf(stderr, "No such file or directory!\nexit code: %d\n", errno);
			exit(EXIT_FAILURE);
		}
		password->length = getline(&password->array,
				&password->length, file);
		cbcDec(&password, key);
		printf("%s", password->array);
	} else if (strcmp(argv[1],"-keygen")==0) {
		key->length = strtol(argv[2], NULL, 10);
		key->array = malloc(key->length*sizeof(key->array[0]));
		keyGen(key);
		printf("%x\n", key->length);
		for (uint16_t i=0;i<key->length;i++) {
			printf("%x\n", key->array[i]);
		}
	} else {
		printf("\nWrong argument!");
		exit(0);
	}

	if (strcmp(argv[1],"-keygen")!=0) {
		free(path);
		free(password);
		fclose(file);
	}
	free(key);
	return 0;
}
