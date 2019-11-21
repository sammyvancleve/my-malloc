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
void swrite(void *ptr);

//16 bytes long
struct allocation {
    uint64_t free;
    uint64_t offset;
};

void *malloc(size_t bytes){
    void *addr;
    //ALL GOOD
    if (bytes % BOUNDARY != 0) {
        bytes = bytes + (BOUNDARY - (bytes % BOUNDARY));
    }
    if (allocstart != NULL) {
        size_t allocsize = bytes + BOOK_SIZE;
        void *check;
        check = allocstart;
        while (check < currentbreak) {
            struct allocation *checkalloc = check;
            if (checkalloc->free == 1) {
                if (checkalloc->offset > allocsize) {
                    checkalloc->free = 0;
                    addr = (char *)check;
                    if ((checkalloc->offset - allocsize) > 32){
                        uint64_t ogoffset = (uint64_t)checkalloc->offset;
                        checkalloc->offset = allocsize;
                        void *nextallocaddr = (char *)check + allocsize;
                        //void *nextallocaddr = (char *)addr + allocsize;
                        struct allocation *nextalloc = nextallocaddr;
                        nextalloc->free = 1;
                        nextalloc->offset = ogoffset - allocsize;
                    }
                    swrite(addr + BOOK_SIZE);
                    return (char *)addr + BOOK_SIZE;
                }
                else if ((uint64_t*)((*checkalloc).offset) == NULL) {
                    if ((check + allocsize + sizeof(checkalloc)) < currentbreak) {
                        addr = check;
                        checkalloc->free = 0;
                        checkalloc->offset = bytes + BOOK_SIZE;

                        //allocation following
                        void *nextallocaddr = (char *)addr + allocsize;
                        struct allocation *nextalloc = nextallocaddr;
                        nextalloc->free = 1;
                        nextalloc->offset = (uint64_t)NULL;
                        return (char *)addr + BOOK_SIZE;
                    } else {
                        break;
                    }
                }
            }
            else if ((uint64_t*)(*checkalloc).offset == NULL) {
                //TODO save this value
                //so next alloc starts here?
                break;
            }
            check = (char *)check + checkalloc->offset;
        }
    }
    if((addr = sbrk((bytes+BOOK_SIZE+BOOK_SIZE))) != (void *) -1) {
        //set allocstart if first time calling malloc
        if (allocstart == NULL){
            allocstart = addr;
        }
        currentbreak = sbrk(0);

        //record all uses of malloc/free
        //make sure they are correct

        //allocation to be returned
        struct allocation *alloc = addr;
        alloc->free = 0;
        alloc->offset = (bytes + BOOK_SIZE);

        //allocation following
        void *nextallocaddr = (char *)addr + alloc->offset;
        struct allocation *nextalloc = nextallocaddr;
        nextalloc->free = 1;
        nextalloc->offset = (uint64_t)NULL;

        //assert((uint64_t )((char *)addr + BOOK_SIZE) % 16 ==0);
        swrite(addr + BOOK_SIZE);
        return (char *)addr + BOOK_SIZE;
    } else {
        return NULL;
    }
}

void free(void *ptr){
    if (ptr != NULL){
        //swrite(ptr);
        uint64_t *free = (uint64_t *)((char *)ptr-BOOK_SIZE);
        *free = 1;
    }
}

void *calloc(size_t nmemb, size_t size){
    void *addr = malloc(nmemb*size);
    memset(addr, 0, nmemb*size);
    //swrite(addr);
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
            swrite(ptr);
            return ptr;
        }
        if (checkalloc->offset == (uint64_t *)NULL){
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

void swrite(void *addr) {
    uint64_t num = (uint64_t)addr;
    char s[50];
    itoa(num, s);
    int fd = open("memory.txt", (O_CREAT | O_RDWR | O_APPEND), S_IRWXU);
    //perror("open: ");
    /*
    if (type == 1) {
        write(fd, "return ", strlen("return "));
    } else if (type == 2) {
        write(fd, "free ", strlen("free "));
    }
    */
    write(fd, s, strlen(s));
    write(fd, "\n", 1);
    //perror("write: ");
    close(fd);
}
