CC=gcc
CFLAGS=-c -Wall

all: shell

shell: commands.o job.o parser.o shell.o
	$(CC) commands.o job.o parser.o shell.o -o shell

commands.o: commands.c commands.h job.h parser.h
	$(CC) $(CFLAGS) commands.c

job.o: job.c commands.h job.h parser.h
	$(CC) $(CFLAGS) job.c

parser.o: parser.c job.h parser.h
	$(CC) $(CFLAGS) parser.c

shell.o: shell.c commands.h job.h parser.h shell.h
	$(CC) $(CFLAGS) shell.c

clean:
	rm -rf *.o shell

