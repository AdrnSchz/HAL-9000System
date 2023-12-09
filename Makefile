all: demo

functions.o: functions.h functions.c
	gcc -Wall -Wextra -g -c functions.c -o functions.o

configs.o: configs.h configs.c
	gcc -Wall -Wextra -g -c configs.c -o configs.o

connections.o: connections.h connections.c
	gcc -Wall -Wextra -g -c connections.c -o connections.o

test.o: test.h test.c
	gcc -Wall -Wextra -g -c test.c -o test.o

bowman.o: bowman.c
	gcc -g -c -Wall -Wextra bowman.c -o bowman.o

poole.o: poole.c
	gcc -g -c -Wall -Wextra poole.c -o poole.o

discovery.o: discovery.c
	gcc -g -c -Wall -Wextra discovery.c -o discovery.o

bowman: bowman.o functions.o test.o configs.o connections.o
	gcc -Wall -Wextra -pthread bowman.o functions.o test.o configs.o connections.o -o bowman

poole: poole.o functions.o test.o configs.o connections.o
	gcc -Wall -Wextra -pthread poole.o functions.o test.o configs.o connections.o -o poole

discovery: discovery.o functions.o test.o configs.o connections.o
	gcc -Wall -Wextra discovery.o functions.o test.o configs.o connections.o -o discovery 

demo: bowman poole discovery

clean:
	rm -fr *.o