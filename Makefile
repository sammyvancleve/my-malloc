CFLAGS=-Wall -pedantic -g -fpic
LDFLAGS=-rdynamic -shared

my-malloc: my-malloc.c
	gcc $(CFLAGS) $(LDFLAGS) -o my-malloc.so my-malloc.c

test-malloc: test-malloc.c
	gcc -g -o test-malloc test-malloc.c

.PHONY: tls 
tls:
	rm memory.txt
	gdb --args env LD_PRELOAD=./my-malloc.so ls -lR /usr/

.PHONY: clean
clean:
	rm -f my-malloc.so

.PHONY: qtest
qtest:
	rm memory.txt
	sudo LD_PRELOAD=./my-malloc.so ls -lR /usr/
	vim memory.txt

.PHONY: qtest2
qtest2:
	sudo LD_PRELOAD=./my-malloc.so ls -l
