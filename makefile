CC = gcc
CFLAGS = -Wall -Wextra -Werror

all: canevas

canevas: canevas.o
	$(CC) $(CFLAGS) -o canevas canevas.o

canevas.o: canevas.c
	$(CC) $(CFLAGS) -c canevas.c

run: all
	./canevas

clean:
	rm *.o canevas
