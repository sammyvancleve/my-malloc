#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define BOUNDARY 16 
#define BOOKKEEP_SIZE 16

void *malloc(size_t bytes);
void free(void *ptr);
static void *allocstart;

//16 bytes long
struct allocation {
    uint64_t free;
    uint64_t offset;
};

void *malloc(size_t bytes){
    void *currentbreak = sbrk(0);
    void *addr;
    if (bytes % BOUNDARY != 0) {
        bytes = bytes + (BOUNDARY - (bytes % BOUNDARY));
    }
    size_t allocsize = bytes + BOOKKEEP_SIZE;
    if (allocstart != NULL){
        struct allocation *checkalloc = allocstart;
        uint64_t checkfree;
        uint64_t checkoffset;
        while ((void *)checkalloc < currentbreak){
            checkfree = (*checkalloc).free;
            checkoffset = (*checkalloc).offset;
            if (checkfree == 1){
                if (checkoffset > allocsize){
                    (*checkalloc).free = 0;
                    addr = (char *)checkalloc + BOOKKEEP_SIZE;
                    return addr;
                }
                //if alloc has not been used before
                else if ((int *)checkoffset == NULL){
                    if ((void *)(checkalloc + allocsize) < currentbreak){
                        (*checkalloc).free = 0;
                        addr = (char *)checkalloc + BOOKKEEP_SIZE;
                        return addr;
                    } else{
                        //TODO how to handle this wasted space!
                        break;
                    }
                }
            }
            else if ((int *)checkoffset == NULL){
                break;
            }
            checkalloc += (*checkalloc).offset;
        }
    }
    //if there is no room for an allocation
    if((addr = sbrk(bytes*4)) != (void *) -1){
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
