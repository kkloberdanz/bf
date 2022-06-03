CC=cc
CFLAGS=--std=c99 -O2 -Wall -Wextra -Wpedantic

bf: bf.c
	$(CC) -o bf bf.c $(CFLAGS)

.PHONY: clean
clean:
	rm -f bf
