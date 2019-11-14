#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define BOUNDARY 16 
#define BOOKKEEP_SIZE 16

void *malloc(size_t bytes);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
static void *allocstart;

//16 bytes long
struct allocation {
    uint64_t free;
    uint64_t offset;
};

void *malloc(size_t bytes){
    void *currentbreak = sbrk(0);
    void *addr;
    //round up size of alloc so it is on boundary
    if (bytes % BOUNDARY != 0) {
        bytes = bytes + (BOUNDARY - (bytes % BOUNDARY));
    }
    if (allocstart != NULL) {
        size_t allocsize = bytes + BOOKKEEP_SIZE;
        struct allocation *checkalloc = allocstart;
        while ((void *)checkalloc < currentbreak) {
            if ((*checkalloc).free == 1) {
                if ((*checkalloc).offset > allocsize) {
                    (*checkalloc).free = 0;
                    addr = (char *)checkalloc + BOOKKEEP_SIZE;
                    return addr;
                }
                else if ((int *)(*checkalloc).offset == NULL) {
                    if ((void *)(checkalloc + allocsize) < currentbreak) {
                        (*checkalloc).free = 0;
                        addr = (char *)checkalloc + BOOKKEEP_SIZE;
                        //TODO set offset properly?
                        return addr;
                    } else {
                        break;
                    }
                }
            }
            else if ((int *)(*checkalloc).offset == NULL) {
                break;
            }
            checkalloc += (*checkalloc).offset;
        }
    }
    if((addr = sbrk(bytes*4)) != (void *) -1) {
        //set allocstart if first time calling malloc
        if (allocstart == NULL){
            allocstart = addr;
        }

        struct allocation *alloc = addr;
        (*alloc).free = 0;
        (*alloc).offset = bytes + BOOKKEEP_SIZE;
        
        void *nextallocaddr = (char *)addr + BOOKKEEP_SIZE +
        (*alloc).offset;
        struct allocation *nextalloc = nextallocaddr;
        (*nextalloc).free = 1;
        (*nextalloc).offset = (uint64_t)NULL;

        return (char *)addr + BOOKKEEP_SIZE;
    } else {
        return NULL;
    }
}

void free(void *ptr){
    if (ptr != NULL){
        uint64_t *free = (uint64_t *)((char *)ptr-BOOKKEEP_SIZE);
        *free = 1;
    }
}

void *calloc(size_t nmemb, size_t size){
    void *addr = malloc(nmemb*size);
    memset(addr, 0, nmemb*size);
    return addr;
}

void *realloc(void *ptr, size_t size){
    void *currentbreak = sbrk(0);
    if (ptr == NULL){
        return malloc(size);
    }
    if (size == 0){
        free(ptr);
    }
    void *checkallocaddr = (char *)ptr - BOOKKEEP_SIZE;
    struct allocation *checkalloc = checkallocaddr;
    size_t currentsize = ((*checkalloc)).offset - BOOKKEEP_SIZE;
    checkalloc += (*checkalloc).offset;
    while ((checkalloc < currentbreak) && ((*checkalloc).free == 1)) {
        currentsize += (*checkalloc).offset;
        if (currentsize >= size){
            return ptr;
        }
    }
    void *addr = malloc(size);
    return addr;
}
