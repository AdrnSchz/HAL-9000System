all: demo

bowman.o: bowman.c
	gcc -g -c bowman.c -o bowman.o

workers.o: poole.c
	gcc -g -c workers.c -o workers.o

demo: bowman.o poole.o
	gcc -Wall -Wextra bowman.o -o bowman
	gcc -Wall -Wextra poole.o -o poole
clean:
	rm -fr main *.o bowman poole