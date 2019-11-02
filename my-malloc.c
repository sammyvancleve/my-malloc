#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

void *mymalloc(int bytes);
void free(void *ptr);
static void *allocstart;

//16 bytes long
struct allocation {
    uint64_t nextallocoffset;
    uint64_t free;
};

int main(int argc, char *argv[]){

    char *s;
    s = mymalloc(15);
    s = "Homer's BBBQ\0";
    char *t;
    t = mymalloc(50);
    t = "The extra B is for BYOBB\0";
    printf("%s\n", s);
    printf("%s\n", t);
}

void *mymalloc(int bytes){
    if (allocstart == NULL){
        allocstart = sbrk(0);
        printf("allocstart %p\n", allocstart);
    }
    void *addr;
    //allocation.free = -1;
    if (bytes % 16 != 0) {
        bytes = bytes + (16 - (bytes % 16));
    }
    if((addr = sbrk(bytes)) != (void *) -1){
        printf("%p\n", (void *)&addr); //testing purposes
        //increment addr by however much space there needs
        //  to be for alloc struct
        void *newbreak = sbrk(0);
        size_t breaksize = newbreak - addr;
        memset(addr, 0, breaksize);
        struct allocation *alloc = addr;
        alloc->free = 0;
        alloc->nextallocoffset = bytes + 16;
        return addr;
    } else {
        return NULL;
    }
}

void free(void *ptr){
    //given memory address ptr, change alloc struct
    //  so that it is free'd
}
