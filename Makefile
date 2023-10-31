all: demo

functions.o: functions.h functions.c
	gcc -g -c functions.c -o functions.o

test.o: test.h test.c
	gcc -g -c test.c -o test.o

bowman.o: bowman.c
	gcc -g -c bowman.c -o bowman.o

poole.o: poole.c
	gcc -g -c poole.c -o poole.o

discovery.o: discovery.c
	gcc -g -c discovery.c -o discovery.o

bowman: bowman.o functions.o test.o
	gcc -Wall -Wextra bowman.o functions.o test.o -o bowman

poole: poole.o functions.o test.o
	gcc -Wall -Wextra poole.o functions.o test.o -o poole

discovery: discovery.o functions.o test.o
	gcc -Wall -Wextra discovery.o functions.o test.o -o discovery 

demo: bowman poole discovery

clean:
	rm -fr bowman poole functions *.o