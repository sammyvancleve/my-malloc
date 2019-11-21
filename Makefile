CFLAGS=-Wall -pedantic -g -fpic
LDFLAGS=-rdynamic -shared

my-malloc: my-malloc.c
	gcc $(CFLAGS) $(LDFLAGS) -o my-malloc.so my-malloc.c

test-malloc: test-malloc.c
	gcc -g -o test-malloc test-malloc.c

.PHONY: tls 
tls:
	gdb --args env LD_PRELOAD=./my-malloc.so ls -lR /usr/

.PHONY: clean
clean:
	rm -f my-malloc.so

.PHONY: qtest
qtest:
	LD_PRELOAD=./my-malloc.so ls -lR /usr/

.PHONY: qtest2
qtest2:
	LD_PRELOAD=./my-malloc.so ls -l
