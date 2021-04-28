output: pmanager.o encryption.o
	gcc pmanager.c encryption.c -o pmanager

install: output
	cp pmanager /usr/bin

dmenu: install
	chmod +x pmanagerDmenu.sh
	cp pmanagerDmenu.sh /usr/bin

pmanager.o: pmanager.c
	gcc -c pmanager.c

encryption.o: encryption.c encryption.h
	gcc -c encryption.c

clean:
	rm *.o pmanager

uninstall:
	rm /usr/bin/pmanager /usr/bin/pmanagerDmenu.sh
