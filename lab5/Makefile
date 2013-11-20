CC = gcc
CFLAGS = -Wall -Wextra -pthread

all: mond

mond: mond.c
	$(CC) $(CFLAGS) -o mond mond.c

webmond: mond.c
	$(CC) $(CFLAGS) -o webmond mond.c

clean:
	rm -f ./*.o mond
