all: demo

functions.o: functions.h functions.c
	gcc -g -c functions.c -o functions.o

bowman.o: bowman.c
	gcc -g -c bowman.c -o bowman.o

poole.o: poole.c
	gcc -g -c poole.c -o poole.o

bowman: bowman.o functions.o
	gcc -Wall -Wextra bowman.o functions.o -o bowman
	
poole: poole.o functions.o
	gcc -Wall -Wextra poole.o functions.o -o poole

demo: bowman poole

clean:
	rm -fr bowman poole functions *.o