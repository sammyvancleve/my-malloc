#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define BOUNDARY 16
#define BOOKKEEPING_SIZE 16

void *mymalloc(int bytes);
void myfree(void *ptr);
static void *allocstart;

//16 bytes long
struct allocation {
    uint64_t free;
    uint64_t offset;
};

int main(int argc, char *argv[]){
    char *s;
    s = mymalloc(70);
    printf("agh %p\n", s);
    strcpy(s, "Homer's bbbq\0");
    printf("agh %p\n", s);
    printf("%s\n", s);
    myfree(s);
    char *t;
    t = mymalloc(50);
    strcpy(t, "The extra B is for BYOBB");
    printf("%s\n", s);
    printf("%s\n", t);
}

//returns pointer of type void *
void *mymalloc(int bytes){
    void *currentbreak = sbrk(0);
    if (bytes % BOUNDARY != 0) {
        bytes = bytes + (BOUNDARY - (bytes % BOUNDARY));
    }
    if (allocstart != NULL){
        struct allocation *checkalloc = allocstart;
        while (checkalloc < currentbreak){
            if (((*checkalloc).free = 1) && (*checkalloc).offset > bytes + 16){
                (*checkalloc).free = 0;
                return (void *)checkalloc + BOOKKEEPING_SIZE;
            } else{
                checkalloc += (*checkalloc).offset;
            }
        }
    }
    void *addr;
    if((addr = sbrk(bytes)) != (void *) -1){
        if (allocstart == NULL){
            allocstart = addr;
        }
        void *newbreak = sbrk(0);
        size_t breaksize = (int *)newbreak - (int *)addr;
        memset(addr, 0, breaksize);
        struct allocation *alloc = addr;
        (*alloc).free = 0;
        (*alloc).offset = bytes + BOOKKEEPING_SIZE;
        return addr + BOOKKEEPING_SIZE;
    } else {
        return NULL;
    }
}

void myfree(void *ptr){
    void *loc = ptr;
    uint64_t *free = loc-BOOKKEEPING_SIZE;
    *free = 1;
}
