#include <unistd.h>
#include <stdio.h>
#include <stdint.h>

int mymalloc(int bytes);
void free(void *ptr);

//8 bytes long
struct alloc {
    uint32_t nextAllocOffset;
    uint32_t free;
};

int main(int argc, char *argv[]){
    char *s;
    s = mymalloc(10);
    s = "poo\0";
    printf("%s\n", s);
}

int mymalloc(int bytes){
    //check if it needs to sbrk more memory...
    //check for empty space 
    //check for allocation that has been freed
    //if it does, then sbrk...
    if (bytes % 1024 != 0){
        bytes = bytes + (1024 - (bytes % 1024));
    }
    int addr = sbrk(bytes);
    //increment addr by however much space there needs
    //  to be for alloc struct
    printf("%p\n", (void *)&addr);

    return addr;
}

void free(void *ptr){
    //given memory address ptr, change alloc struct
    //  so that it is free'd
}
