#include <unistd.h>
#include <stdio.h>

int mymalloc(int bytes);

int main(int argc, char *argv[]){

    char *s;
    s = mymalloc(10);
    s = "poo\0";
    printf("%s\n", s);
}

int mymalloc(int bytes){
    int addr = sbrk(bytes);
    printf("%p\n", (void *)&addr);

    return addr;
}
