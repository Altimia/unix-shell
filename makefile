q2: main.o token.o command.o
	gcc -Wall main.o token.o command.o -o shell 

main.o: main.c
	gcc -Wall -c main.c

command.o: command.c command.h
	gcc -Wall -c command.c

token.o: token.c token.h
	gcc -Wall -c token.c

clean:
	rm *.o
