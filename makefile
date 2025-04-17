CC = gcc


all: canevas

canevas: canevas.o
	$(CC) -o canevas canevas.o

canevas.o: canevas.c
	$(CC) -c canevas.c

run: all
	./canevas

clean:
	rm *.o canevas
