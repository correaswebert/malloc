# TODO: Generalise it later

all:
	gcc -Wall -I include -o build/malloc.o -c src/malloc.c
	gcc -Wall -I include -o build/test_malloc.c test/malloc.c build/malloc.o