#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define BOUNDARY 16 
#define BOOK_SIZE 16

static void *allocstart;
static void *currentbreak;

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
        struct allocation *checkalloc;
        while (check < currentbreak) {
            checkalloc = check;
            if (checkalloc->free == 1) {
                if (checkalloc->offset > allocsize) {
                    checkalloc->free = 0;
                    addr = check + BOOK_SIZE;
                    return addr;
                }
                else if ((uint64_t *)(*checkalloc).offset == NULL) {
                    if ((check + allocsize + sizeof(checkalloc)) < currentbreak) {
                        addr = check;
                        checkalloc->free = 0;
                        checkalloc->offset = allocsize;

                        //allocation following
                        void *nextallocaddr = addr + checkalloc->offset;
                        struct allocation *nextalloc = nextallocaddr;
                        nextalloc->free = 1;
                        nextalloc->offset = (uint64_t)NULL;

                        return addr + BOOK_SIZE;
                    } else {
                        break;
                    }
                }
            }
            //if its free and theres no offset
            else if ((uint64_t*)(*checkalloc).offset == NULL) {
                break;
            }
            check = check + checkalloc->offset;
        }
    }
    if((addr = sbrk(bytes+10240)) != (void *) -1) {
        //set allocstart if first time calling malloc
        if (allocstart == NULL){
            allocstart = addr;
        }
        currentbreak = sbrk(0);

        //allocation to be returned
        struct allocation *alloc = addr;
        alloc->free = 0;
        alloc->offset = bytes + BOOK_SIZE;

        //allocation following
        void *nextallocaddr = (char *)addr + alloc->offset;
        struct allocation *nextalloc = nextallocaddr;
        nextalloc->free = 1;
        nextalloc->offset = (uint64_t)NULL;

        return (char *)addr + BOOK_SIZE;
    } else {
        return NULL;
    }
}

void free(void *ptr){
    if (ptr != NULL){
        uint64_t *free = (uint64_t *)((char *)ptr-BOOK_SIZE);
        *free = 1;
    }
}

void *calloc(size_t nmemb, size_t size){
    void *addr = malloc(nmemb*size);
    memset(addr, 0, nmemb*size);
    return addr;
}

void *realloc(void *ptr, size_t size){
    if (ptr == NULL){
        return malloc(size);
    }
    if (size == 0){
        free(ptr);
    }
    void *checkallocaddr = (char *)ptr - BOOK_SIZE;
    struct allocation *checkalloc = checkallocaddr;
    size_t currentsize = checkalloc->offset - BOOK_SIZE;
    checkallocaddr += checkalloc->offset;
    while ((checkallocaddr < currentbreak) && (checkalloc->free == 1)) {
        checkalloc = checkallocaddr;
        currentsize += checkalloc->offset;
        if (currentsize >= size){
            return ptr;
        }
        checkallocaddr += checkalloc->offset;
    }
    void *addr = malloc(size);
    return addr;
}
