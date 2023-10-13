all: demo

bowman.o: bowman.c
	gcc -g -c bowman.c -o bowman.o

poole.o: poole.c
	gcc -g -c poole.c -o poole.o

demo: bowman.o poole.o
	gcc -Wall -Wextra bowman.o -o bowman
	gcc -Wall -Wextra poole.o -o poole
clean:
	rm -fr bowman poole *.o 