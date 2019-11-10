#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define BOUNDARY 16 
#define BOOKKEEPING_SIZE 16
#define SBRK_INCR 4096

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
    void *currentbreak = sbrk(0);
    if (bytes % BOUNDARY != 0) {
        bytes = bytes + (BOUNDARY - (bytes % BOUNDARY));
    }
    if (allocstart != NULL){
        struct allocation *checkalloc = allocstart;
        while ((void *)checkalloc < currentbreak){
            if (((*checkalloc).free = 1) && (*checkalloc).offset > bytes + 16){
                (*checkalloc).free = 0;
                return (void *)checkalloc + BOOKKEEPING_SIZE;
            } else{
                checkalloc += (*checkalloc).offset;
            }
        }
    }
    void *addr;
    if((addr = sbrk(bytes*4)) != (void *) -1){
        if (allocstart == NULL){
            allocstart = addr;
        }
        void *newbreak = sbrk(0);
        int breaksize = (int *)newbreak - (int *)addr;
        //memset(addr, 0, breaksize);
        struct allocation *alloc = addr;
        (*alloc).free = 0;
        (*alloc).offset = bytes + BOOKKEEPING_SIZE;
        struct allocation *nextalloc = (char *)addr + BOOKKEEPING_SIZE +
        (*alloc).offset;
        (*nextalloc).free = 1;
        (*nextalloc).offset = NULL;
        return addr + BOOKKEEPING_SIZE;
    } else {
        return NULL;
    }
}

void free(void *ptr){
    void *loc = ptr;
    uint64_t *free = loc-BOOKKEEPING_SIZE;
    *free = 1;
}
