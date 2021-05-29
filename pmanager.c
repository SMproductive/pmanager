/*
   Sat 29 May 2021 04:30:46 PM CEST
   Developed by Maximilian Strele

   Description
   Passwordmanager using it's own encryption algorithm
*/
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <gcrypt.h>
#include <gtk/gtk.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <termios.h>
#include <time.h>

#include "encryption.h"

static void setKey();
static void getPassWordShell(char **, size_t *, FILE *);
static void passWordGen(struct messageArray *);
static void clipHandling();
static void guiActivate();

/* Variables */
GtkApplication *app;
GtkWidget *window;
GtkWidget *grid;
GtkWidget *label;
GtkWidget *buffer;
GtkWidget *entry;
static struct keyArray *key;
static struct messageArray *password;
static FILE *file;
static char *path;
static char *masterPassword;

/* Function implementations */
void
setKey()
{
	/* Variables */
	gcry_md_hd_t handler;

	/* Logic */
	key->length = gcry_md_get_algo_dlen(GCRY_MD_SHA512);
	key->array = malloc(key->length*sizeof(key->array[0]));

	gcry_md_open(&handler, GCRY_MD_SHA512, GCRY_MD_FLAG_SECURE);
	gcry_md_write(handler, masterPassword, strlen(masterPassword));
	key->array = gcry_md_read(handler, GCRY_MD_SHA512);
}

void
getPassWordShell(char **lineptr, size_t *n, FILE *stream)
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
	for (size_t i=0;i<password->length-1;i++) {
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
clipHandling()
{
	masterPassword = gtk_entry_buffer_get_text(buffer);
	setKey();
	g_print("%s ", masterPassword);
	g_print("\n%s ", key->array);
	g_print("\n%s ", path);
	file = fopen(path, "r");
	password->length = getline((char**)&password->array, &password->length, file);
	g_print("\n%d ", password->length);
	cbcDec(&password, key);


}

void
guiActivate()
{
	/* Logic */
	window = gtk_application_window_new(app);
	grid = gtk_grid_new();
	label = gtk_label_new("password: ");
	entry = gtk_entry_new();
	buffer = gtk_entry_buffer_new(NULL, 0);

	gtk_entry_set_visibility(GTK_ENTRY (entry), false);
	gtk_entry_set_buffer(entry, buffer);
	g_signal_connect(entry, "activate", G_CALLBACK (clipHandling), NULL);

	gtk_window_set_title(GTK_WINDOW (window), "pmanager");
	gtk_window_set_default_size(GTK_WINDOW (window), 200, 200);
	gtk_window_set_child(GTK_WINDOW (window), grid);

	gtk_grid_attach(GTK_GRID (grid), label, 1, 1, 1, 1);
	gtk_grid_attach(GTK_GRID (grid), entry, 1, 2, 1, 1);

	gtk_widget_show(window);

}

int
main(int argc, char *argv[])
{
	/* Variables */
	size_t pathLen;
	size_t tempLen;

	/* Logic */
	if (argc<3) {
		printf("Too few arguments!");
		exit(0);
	}
	key = calloc(1, sizeof(*key));
	path = NULL;
	password = calloc(1, sizeof(*password));
	pathLen = strlen(getenv("HOME"));
	pathLen += strlen("/.pmanager/");
	pathLen += strlen(argv[argc-1]);
	path = malloc(pathLen*sizeof(*path));
	strcat(path, getenv("HOME"));
	strcat(path, "/.pmanager/");
	mkdir(path, 6600);
	strcat(path, argv[argc-1]);

	if (strcmp(argv[1],"-set")==0) {
		printf("masterPassword: ");
		getPassWordShell(&masterPassword, &tempLen, stdin);
		setKey();
		printf("\npassword ('str'): ");
		getPassWordShell((char**)&password->array, &password->length, stdin);
		cbcEnc(&password, key);
		printf("\nlength: %d", password->length);
		file = fopen(path, "w");
		if (file==NULL) {
			fprintf(stderr, "Permission denied!\nexit code: %d\n", errno);
			exit(EXIT_FAILURE);
		}
		for (size_t i=0;i<password->length;i++) {
			fputc(password->array[i], file);
		}
		fclose(file);

	} else if (strcmp(argv[1],"-gen")==0) {
		if (argc<4) {
			printf("Too few arguments at program start!");
			exit(0);
		}
		printf("masterPassword: ");
		getPassWordShell(&masterPassword, &tempLen, stdin);
		setKey();
		password->length = strtol(argv[2], NULL, 10)+1;
		password->array = malloc(
				password->length*sizeof(password->array[0]));
		printf("\nseed (length=passwordlength): ");
		passWordGen(password);
		cbcEnc(&password, key);
		file = fopen(path, "w");
		if (file==NULL) {
			fprintf(stderr, "Permission denied!\nexit code: %d\n", errno);
			exit(EXIT_FAILURE);
		}
		for (size_t i=0;i<password->length;i++) {
			fputc(password->array[i], file);
		}
		fclose(file);

	} else if (strcmp(argv[1],"-get")==0) {
		app = gtk_application_new("pmanager.gui", G_APPLICATION_FLAGS_NONE);
		g_signal_connect(app, "activate", G_CALLBACK (guiActivate), NULL);
		g_application_run(G_APPLICATION (app), 0, NULL);
/*		printf("masterPassword: ");
		getPassWordShell(&masterPassword, &tempLen, stdin);
		setKey();
		file = fopen(path, "r");
		if (file==NULL) {
			fprintf(stderr, "Permission denied or file does not exist!\nexit code: %d\n", errno);
			exit(EXIT_FAILURE);
		}
		password->length = getline((char**)&password->array, &password->length, file);
		fclose(file);
		cbcDec(&password, key);
		for (size_t i=0;i<password->length;i++) {
			printf("%c", password->array[i]);
		}
*/
	} else {
		printf("\nWrong argument!");
		exit(0);
	}
	free(path);
	free(password);
	free(key);
	exit(0);
}
