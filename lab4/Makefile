CC = gcc
CFLAGS = -Wall -Wextra

all: mond example

mond: mond.c
	$(CC) $(CFLAGS) -o mond mond.c

example: ten.c
	$(CC) $(CFLAGS) -o example ten.c

clean:
	rm -f ./*.o mond example
