/*
   Tue 27 Apr 2021 06:01:00 PM CEST
   Developed by Maximilian Strele

   Description
   Passwordmanager using it's own encryption algorithm
   TODO password remove
*/

#include <errno.h>
#include <gcrypt.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <math.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <termios.h>
#include <time.h>

#include "encryption.h"

/* Function declarations */
static void gui();
static void delay(void *);
static void setKey();
static void copyToClipboard();
static void getPassWordShell(char **, int *, FILE *);
static void passWordGen(struct messageArray *);
static void keyGen(struct keyArray *);
static void get();

/* Variables */
static GtkApplication *app;
static GdkDisplay *display;
static GdkClipboard *clip;
static GtkWidget *window;
static GtkWidget *box;
static GtkWidget *label;
static GtkWidget *passwordEntry;
static GtkEntryBuffer *buffer;
static struct keyArray *key;
static struct messageArray *password;
static FILE *file;
static char *path;
static uint8_t inClipBoard;
static uint8_t waiting;
static char *masterPassword;

/* Function implementations */
void gui()
{
	window = gtk_application_window_new(app);
	gtk_window_set_resizable(window, false);
	gtk_window_set_title(GTK_WINDOW (window), "pmanager");
	gtk_window_set_default_size(GTK_WINDOW (window), 200, 50);

	box = gtk_center_box_new();
	gtk_window_set_child(GTK_WINDOW (window), box);

	display = gdk_display_get_default();
	clip = gdk_display_get_clipboard(display);

	label = gtk_label_new("Password: ");
	buffer = gtk_entry_buffer_new("", 0);
	passwordEntry = gtk_entry_new();
	gtk_entry_set_buffer(passwordEntry, buffer);
	gtk_entry_set_visibility(passwordEntry, FALSE);

	g_signal_connect(passwordEntry, "activate", G_CALLBACK (gtk_widget_hide), window);

	g_signal_connect(window, "hide", G_CALLBACK (copyToClipboard), NULL);

	gtk_center_box_set_start_widget(GTK_CENTER_BOX (box), label);
	gtk_center_box_set_center_widget(GTK_CENTER_BOX (box), passwordEntry);

	gtk_widget_show(window);
}

void
delay(void *waitTime)
{
	/* Variables */
	unsigned int startTime;
	unsigned int wait;

	/* Logic */
	wait = (unsigned int)waitTime;
	startTime = time(0);
	while (time(0)<=startTime+waitTime);
	waiting = 0;
}

void
setKey()
{
	/* Variables */
	gcry_md_hd_t handler;

	/* Logic */
	key->length = gcry_md_get_algo_dlen(GCRY_MD_SHA3_384);
	key->array = malloc(key->length*sizeof(key->array[0]));

	gcry_md_open(&handler, GCRY_MD_SHA3_384, GCRY_MD_FLAG_SECURE);
	gcry_md_write(handler, masterPassword, strlen(masterPassword));
	key->array = gcry_md_read(handler, GCRY_MD_SHA3_384);
}

void
copyToClipboard()
{
	/* Logic */
	inClipBoard = 1;
	masterPassword = gtk_entry_buffer_get_text(buffer);
	setKey();
	cbcDec(&password, key);
	gdk_clipboard_set_text(clip, password->array);
}

void
getPassWordShell(char **lineptr, int *n, FILE *stream)
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

void
get()
{
	/* Logic */
	app = gtk_application_new("pmanager.p", G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK (gui), NULL);
	g_application_run(G_APPLICATION (app), 0, NULL);
	g_object_unref(app);

}

int
main(int argc, char *argv[])
{
	/* Variables */
	int pathLen;

	/* Logic */
	if (argc<3) {
		printf("Too few arguments!");
		exit(0);
	}
	key = malloc(sizeof(*key));

	if (strcmp(argv[1],"-keygen")!=0) {
		path = NULL;
		password = malloc(sizeof(*password));
		pathLen = strlen(getenv("HOME"));
		pathLen += strlen("/.pmanager/");
		pathLen += strlen(argv[argc-1]);
		path = malloc(pathLen*sizeof(*path));
		strcat(path, getenv("HOME"));
		strcat(path, "/.pmanager/");
		mkdir(path, 6600);
		strcat(path, argv[argc-1]);
	}

	if (strcmp(argv[1],"-set")==0) {
		printf("masterPassword: ");
		getPassWordShell(&masterPassword, NULL, stdin);
		setKey();
		printf("\npassword ('str'): ");
		getPassWordShell(&password->array, &password->length, stdin);
		cbcEnc(&password, key);
		file = fopen(path, "w");
		if (file==NULL) {
			fprintf(stderr, "Permission denied!\nexit code: %d\n", errno);
			exit(EXIT_FAILURE);
		}
		fputs(password->array, file);
		fclose(file);
	} else if (strcmp(argv[1],"-gen")==0) {
		if (argc<4) {
			printf("Too few arguments at program start!");
			exit(0);
		}
		printf("masterPassword: ");
		getPassWordShell(&masterPassword, NULL, stdin);
		setKey();
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
		fclose(file);
	} else if (strcmp(argv[1],"-get")==0) {
		uint8_t delayCreated;
		pthread_t threads[2];
		file = fopen(path, "r");
		if (file==NULL) {
			fprintf(stderr, "No such file or directory!\nexit code: %d\n",
					errno);
			exit(EXIT_FAILURE);
		}
		password->length = getline(&password->array,
				&password->length, file);
		fclose(file);
		waiting = 1;
		delayCreated = 0;
		pthread_create(&threads[0], NULL, get, NULL);
		while (waiting) {
			if (inClipBoard&&!delayCreated) {
				pthread_create(&threads[1], NULL, delay, (void*)30);
			}
		}
		exit(0);




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
	}
	free(key);
	return 0;
}
