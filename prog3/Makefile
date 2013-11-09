CC = gcc
CFLAGS = -Wall -Wextra -pthread

all: mond

mond: mond.c
	$(CC) $(CFLAGS) -o mond mond.c

example: ten.c
	$(CC) $(CFLAGS) -o example ten.c

clea:
	rm -f ./*.o mond

clean:
	rm -f ./*.o mond
