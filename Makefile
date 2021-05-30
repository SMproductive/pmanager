CC = gcc 
CFLAGS = -std=c99 -Os 
EXTERNFLAGS = `pkg-config --cflags gtk4` `libgcrypt-config --cflags`
LIBS = `libgcrypt-config --libs` `pkg-config --libs gtk4`

output: pmanager.o encryption.o
	${CC} ${CFLAGS} ${EXTERNFLAGS} ${LIBS} pmanager.c encryption.c -o pmanager

install: output
	cp pmanager /usr/bin
	chmod 755 /usr/bin/pmanager
	cp pmanager.1 /usr/local/share/man/man1/

pmanager.o: pmanager.c
	${CC} ${CFLAGS} ${EXTERNFLAGS} ${LIBS} -c pmanager.c

encryption.o: encryption.c encryption.h
	${CC} ${CFLAGS} -c encryption.c

dmenu: install
	chmod 755 pmanagerDmenu.sh
	cp pmanagerDmenu.sh /usr/bin

clean:
	rm *.o pmanager

uninstall:
	rm /usr/bin/pmanager /usr/bin/pmanagerDmenu.sh
