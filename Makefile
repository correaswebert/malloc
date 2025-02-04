# TODO: Generalise it later

all:
	gcc -g -Wall -I include -o build/console.o -c src/console.c
	gcc -g -Wall -I include -o build/malloc.o -c src/malloc.c
	gcc -g -Wall -I include -o build/test_malloc \
		build/console.o \
		build/malloc.o \
		test/malloc.c