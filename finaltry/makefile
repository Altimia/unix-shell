CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99
TARGET = myshell
OBJS = commands.o job.o parser.o shell.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

commands.o: commands.c commands.h
	$(CC) $(CFLAGS) -c commands.c

job.o: job.c job.h parser.h
	$(CC) $(CFLAGS) -c job.c

parser.o: parser.c parser.h
	$(CC) $(CFLAGS) -c parser.c

shell.o: shell.c commands.h job.h parser.h
	$(CC) $(CFLAGS) -c shell.c

clean:
	rm -f $(TARGET) $(OBJS)
