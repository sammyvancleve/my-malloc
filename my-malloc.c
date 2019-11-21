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
    //round up size of alloc so it is on boundary
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
                    //if ((checkalloc->offset - allocsize) > 32){
                        //checkalloc->offset = allocsize
                    //}
                    return addr + BOOK_SIZE;
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
            //if its free and theres no offset
            else if ((uint64_t*)(*checkalloc).offset == NULL) {
                break;
            }
            check = (char *)check + checkalloc->offset;
        }
    }
    if((addr = sbrk((bytes+BOOK_SIZE+BOOK_SIZE)+4096000)) != (void *) -1) {
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

        assert((uint64_t )((char *)addr + BOOK_SIZE) % 16 ==0);
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
    while ((checkallocaddr < currentbreak) && (checkalloc->free == 1) &&
    (checkalloc->offset != (uint64_t)NULL)) {
        checkalloc = checkallocaddr;
        currentsize += (size_t)checkalloc->offset;
        if (currentsize >= size){
            struct allocation *returnalloc = ptr;
            returnalloc->offset = (uint64_t)currentsize + BOOK_SIZE;
            //swrite(ptr);
            return ptr;
        }
        checkallocaddr = (char *)checkallocaddr + checkalloc->offset;
    }
    return malloc(size);
}

void swrite(void *addr) {
    int fd = open("memory.txt", O_RDWR | O_CREAT, S_IRWXU);
    //perror("open: ");
    //char *s = (uint64_t)&addr;
    write(fd, (char *)&addr, strlen(sizeof(addr)));
    write(fd, "\n", 1);
    close(fd);
}
