CFLAGS=-Wall -pedantic -g -fpic
LDFLAGS=-rdynamic -shared

my-malloc: my-malloc.c
	gcc $(CFLAGS) $(LDFLAGS) -o my-malloc2.so my-malloc2.c

test-malloc: test-malloc.c
	gcc -g -o test-malloc test-malloc.c

.PHONY: tls 
tls:
	touch memory.txt
	rm memory.txt
	gdb --args env LD_PRELOAD=./my-malloc2.so ls -lR /usr/

.PHONY: clean
clean:
	rm -f my-malloc2.so

.PHONY: qtest
qtest:
	touch memory.txt
	rm memory.txt
	sudo LD_PRELOAD=./my-malloc2.so ls -lR /usr/
	vim memory.txt

.PHONY: qtest2
qtest2:
	sudo LD_PRELOAD=./my-malloc2.so ls -l
