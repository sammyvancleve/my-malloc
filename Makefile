CFLAGS=-Wall -pedantic -g -rdynamic -shared -fpic

my-malloc: my-malloc.c
	gcc $(CFLAGS) -o my-malloc.so my-malloc.c

test-malloc: test-malloc.c
	gcc -g -o test-malloc test-malloc.c

.PHONY: testls
testls:
	gdb --args env LD_PRELOAD=./my-malloc.so ls -l

.PHONY: clean
clean:
	rm -f my-malloc.so
