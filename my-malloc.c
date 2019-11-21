#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define BOUNDARY 16 
#define BOOK_SIZE 16

static void *allocstart;
static void *currentbreak;
static void *next;
void swrite(void *ptr, int type, int func);

//16 bytes long
struct allocation {
    uint64_t free;
    uint64_t offset;
};

void *search(size_t bytes) {
    size_t allocsize = bytes + BOOK_SIZE;
    void *check = allocstart;
    struct allocation *checkalloc;
    while (check < currentbreak) {
        checkalloc = check;
        if (checkalloc->free == 1 && checkalloc->offset >= allocsize) {
            checkalloc->free = 0;
            return check;
        } else if ((uint64_t *)checkalloc->offset == NULL) {
            void *nextallocspace = (char *)check + allocsize + BOOK_SIZE;
            if (currentbreak > nextallocspace) {
                checkalloc->free = 0;
                checkalloc->offset = allocsize;
                
                void *nextallocaddr = (char *)check + checkalloc->offset;
                struct allocation *nextalloc = nextallocaddr;
                nextalloc->free = 1;
                nextalloc->offset = (uint64_t)NULL;

                return check;
            } else {
                next = check;
                return NULL;
            }
        }
        check = (char *)check + checkalloc->offset;
    }
    return NULL;
}

void *malloc(size_t bytes){
    void *addr;
    if (bytes % BOUNDARY != 0) {
        bytes = bytes + (BOUNDARY - (bytes % BOUNDARY));
    }
    size_t minimumallocsize = bytes + (BOOK_SIZE*2);
    if (allocstart != NULL) {
        if ((addr = search(bytes)) != NULL) {
            swrite(addr + BOOK_SIZE, 1, 1);
            return (char *)addr + BOOK_SIZE;
        }
    }
    if((addr = sbrk(minimumallocsize*4)) != (void *) -1) {
        //set allocstart if first time calling malloc
        if (allocstart == NULL){
            allocstart = addr;
        }
        if (next != NULL){
            addr = next;
            next = NULL;
        }
        currentbreak = sbrk(0);

        //allocation to be returned
        struct allocation *alloc = addr;
        alloc->free = 0;
        alloc->offset = (bytes + BOOK_SIZE);

        //allocation following
        void *nextallocaddr = (char *)addr + alloc->offset;
        struct allocation *nextalloc = nextallocaddr;
        nextalloc->free = 1;
        nextalloc->offset = (uint64_t)NULL;

        swrite(addr + BOOK_SIZE, 1, 1);
        return (char *)addr + BOOK_SIZE;
    } else {
        return NULL;
    }
}

void free(void *ptr){
    if (ptr != NULL){
        swrite(ptr, 2, 4);
        uint64_t *free = (uint64_t *)((char *)ptr-BOOK_SIZE);
        *free = 1;
    }
}

void *calloc(size_t nmemb, size_t size){
    void *addr = malloc(nmemb*size);
    memset(addr, 0, nmemb*size);
    swrite(addr, 1, 2);
    return addr;
}

void *realloc(void *ptr, size_t size){
    if (ptr == NULL) {
        return malloc(size);
    }
    if (size == 0) {
        free(ptr);
    }
    //create starting address
    void *checkallocaddr = (char *)ptr - BOOK_SIZE;
    struct allocation *checkalloc = checkallocaddr;
    //get current size
    size_t currentsize = checkalloc->offset - BOOK_SIZE;
    checkallocaddr = (char *)checkallocaddr + checkalloc->offset;
    if (checkallocaddr < currentbreak) {
        checkalloc = checkallocaddr;
    } else {
        return malloc(size);
    }
    while ((checkallocaddr < currentbreak) && (checkalloc->free == 1)) {
        checkalloc = checkallocaddr;
        currentsize += (size_t)checkalloc->offset;
        if (currentsize >= size){
            struct allocation *returnalloc = ptr;
            returnalloc->offset = (uint64_t)currentsize + BOOK_SIZE;
            swrite(ptr, 1, 3);
            return ptr;
        }
        if (checkalloc->offset == (uint64_t)NULL){
            break;
        }
        checkallocaddr = (char *)checkallocaddr + checkalloc->offset;
    }
    return malloc(size);
}

void itoa(uint64_t n, char s[]){
    uint64_t i = 0;
    do {
        s[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);
    char reverses[50];
    for (int x = 0; x < i; x++){
        reverses[x] = s[i-1-x];
    }
    for (int y = 0; y < i; y++){
        s[y] = reverses[y];
    }
    s[i] = '\0';
}

void swrite(void *addr, int type, int func) {
    void *allocaddress = (char *)addr - BOOK_SIZE;
    struct allocation *alloc = allocaddress;
    uint64_t address = (uint64_t)addr;
    uint64_t addr2 = (uint64_t)addr + alloc->offset;
    char addrstring[50];
    itoa(address , addrstring);
    char s2[50];
    itoa(alloc->offset, s2);
    char addr2string[50];
    itoa(addr2, addr2string);
    int fd = open("memory.txt", O_CREAT|O_WRONLY|O_APPEND, 0777);
    if (func == 1){
        write(fd, "malloc ", strlen("malloc "));
    } else if (func == 2) {
        write(fd, "calloc ", strlen("calloc "));
    } else if (func == 3) {
        write(fd, "realloc ", strlen("realloc "));
    } else if (func == 4) {
        write(fd, "free ", strlen("free "));
    }
    /*if (type == 1) {
        write(fd, "return ", strlen("return "));
    } else if (type == 2) {
        write(fd, "free ", strlen("free "));
    }*/
    write(fd, addrstring, strlen(addrstring));
    write(fd, " -> ", 4);
    write(fd, s2, strlen(s2));
    write(fd, " -> ", 4);
    write(fd, addr2string, strlen(addr2string));
    write(fd, "\n", 1);
    if (close(fd) == -1){
        perror("close: ");
    }
}
