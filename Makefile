CC = gcc 
CFLAGS = -std=c99 -Os 
EXTERNFLAGS = `pkg-config --cflags gtk4` `libgcrypt-config --cflags`
LIBS = `libgcrypt-config --libs` `pkg-config --libs gtk4`

output: pmanager_without_Gui.o encryption.o
	${CC} ${CFLAGS} ${EXTERNFLAGS} ${LIBS} pmanager_without_Gui.c encryption.c -o pmanager

install: output
	cp pmanager /usr/bin

dmenu: install
	chmod +x pmanagerDmenu.sh
	cp pmanagerDmenu.sh /usr/bin

pmanager_without_Gui.o: pmanager_without_Gui.c
	${CC} ${CFLAGS} ${EXTERNFLAGS} ${LIBS} -c pmanager_without_Gui.c

encryption.o: encryption.c encryption.h
	${CC} ${CFLAGS} -c encryption.c

clean:
	rm *.o pmanager

uninstall:
	rm /usr/bin/pmanager /usr/bin/pmanagerDmenu.sh
