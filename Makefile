all: demo

functions.o: functions.h functions.c
	gcc -Wall -Wextra -g -c functions.c -o functions.o

configs.o: configs.h configs.c
	gcc -Wall -Wextra -g -c configs.c -o configs.o

test.o: test.h test.c
	gcc -Wall -Wextra -g -c test.c -o test.o

bowman.o: bowman.c
	gcc -Wall -Wextra -g -c bowman.c -o bowman.o

poole.o: poole.c
	gcc -Wall -Wextra -g -c poole.c -o poole.o

discovery.o: discovery.c
	gcc -Wall -Wextra -g -c discovery.c -o discovery.o

bowman: bowman.o functions.o test.o configs.o
	gcc -Wall -Wextra bowman.o functions.o test.o configs.o -o bowman

poole: poole.o functions.o test.o configs.o
	gcc -Wall -Wextra poole.o functions.o test.o configs.o -o poole

discovery: discovery.o functions.o test.o configs.o
	gcc -Wall -Wextra discovery.o functions.o test.o configs.o -o discovery 

demo: bowman poole discovery

clean:
	rm -fr bowman poole functions configs test *.o