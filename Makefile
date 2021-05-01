CC = gcc
CFLAGS = -std=c99 -Os

output: pmanager.o encryption.o
	${CC} ${CFLAGS} pmanager.c encryption.c -o pmanager

install: output
	cp pmanager /usr/bin

dmenu: install
	chmod +x pmanagerDmenu.sh
	cp pmanagerDmenu.sh /usr/bin

pmanager.o: pmanager.c
	${CC} ${CFLAGS} -c pmanager.c

encryption.o: encryption.c encryption.h
	${CC} ${CFLAGS} -c encryption.c

clean:
	rm *.o pmanager

uninstall:
	rm /usr/bin/pmanager /usr/bin/pmanagerDmenu.sh
