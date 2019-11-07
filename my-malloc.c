#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

void *mymalloc(int bytes);
void free(void *ptr);
static void *allocstart;

//16 bytes long
struct allocation {
    uint64_t free;
    uint64_t offset;
};

int main(int argc, char *argv[]){
    char *s;
    s = mymalloc(51);
    *s = "Homer's BBBQ\0";
    free(s);
    char *t;
    t = mymalloc(50);
    *t = "The extra B is for BYOBB\0";
    printf("%s\n", s);
    printf("%s\n", t);
}

void *mymalloc(int bytes){
    void *currentbreak = sbrk(0);
    if (bytes % 16 != 0) {
        bytes = bytes + (16 - (bytes % 16));
    }
    if (allocstart != NULL){
        struct allocation *checkalloc = allocstart;
        printf("checkalloc free %ld offset %ld\n", (*checkalloc).free,
        (*checkalloc).offset);
        while (checkalloc < currentbreak){
            if (((*checkalloc).free = 1) && (*checkalloc).offset < bytes + 16){
                checkalloc->free = 0;
                return checkalloc + 16;
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
        (*alloc).offset = bytes + 16;
        printf("address is %p\n", addr);
        return addr + 16;
    } else {
        return NULL;
    }
}

void free(void *ptr){
    uint64_t *free = ptr-16;
    *free = 1;
}
