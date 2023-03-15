CC = gcc
CFLAGS = -Wall -Werror -g
PROGS = minget minls
OBJS = minget.o minls.o minhelper.o

all: $(PROGS)

minget: minget.o minhelper.o
	$(CC) $(CFLAGS) -o minget minget.o minhelper.o
minls: minls.o minhelper.o
	$(CC) $(CFLAGS) -o minls minls.o minhelper.o


clean:
	rm -f $(OBJS) *~ TAGS
