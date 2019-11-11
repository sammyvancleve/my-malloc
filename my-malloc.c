#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define BOUNDARY 16 
#define BOOKKEEPING_SIZE 16

void *malloc(size_t bytes);
void free(void *ptr);
static void *allocstart;

//16 bytes long
struct allocation {
    uint64_t free;
    uint64_t offset;
};

int main(int argc, char *argv[]){
/*
    char *s;
    s = malloc(10000);
    strcpy(s, "Homer's BBBQ\0");
    printf("%s\n", s);
    free(s);
    char *t;
    t = malloc(50);
    strcpy(t, "The extra B is for BYOBBB\0");
    printf("%s\n", s);
    printf("%s\n", t);*/
}

//returns pointer of type void *
void *malloc(size_t bytes){
    //break at instant malloc is run
    void *currentbreak = sbrk(0);
    //address to be returned
    void *addr;
    if (bytes % BOUNDARY != 0) {
        bytes = bytes + (BOUNDARY - (bytes % BOUNDARY));
    }
    //size needed for allocation
    size_t allocsize = bytes + BOOKKEEPING_SIZE;
    //if malloc has been run once before
    if (allocstart != NULL){
        struct allocation *checkalloc = allocstart;
        uint64_t checkfree;
        uint64_t checkoffset;
        while ((void *)checkalloc < currentbreak){
            checkfree = (*checkalloc).free;
            checkoffset = (*checkalloc).offset;
            if (checkfree == 1){
                //if enough space in alloc
                if (checkoffset > bytes + 16){
                    (*checkalloc).free = 0;
                    addr = (char *)checkalloc + BOOKKEEPING_SIZE;
                    return addr;
                }
                //if alloc has not been used before
                else if (checkoffset == NULL){
                    if (((char *)checkalloc + allocsize) < currentbreak){
                        (*checkalloc).free = 0;
                        addr = (char *)checkalloc + BOOKKEEPING_SIZE;
                        return addr;
                    } else{
                        break;
                    }
                }
            }
            else{
                if (checkoffset != NULL){
                    checkalloc += (*checkalloc).offset;
                } else{
                    break;
                }
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
        void *newbreak = sbrk(0);
        int breaksize = (int *)newbreak - (int *)addr;
        //memset(addr, 0, breaksize);
        struct allocation *alloc = addr;
        (*alloc).free = 0;
        (*alloc).offset = bytes + BOOKKEEPING_SIZE;
        void *nextallocaddr = (char *)addr + BOOKKEEPING_SIZE +
        (*alloc).offset;
        struct allocation *nextalloc = nextallocaddr;
        (*nextalloc).free = 1;
        (*nextalloc).offset = NULL;
        return (char *)addr + BOOKKEEPING_SIZE;
    } else {
        return NULL;
    }
}

void free(void *ptr){
    if (ptr != NULL){
        void *loc = ptr;
        uint64_t *free = loc-BOOKKEEPING_SIZE;
        *free = 1;
    }
}
