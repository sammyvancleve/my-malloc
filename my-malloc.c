#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define BOUNDARY 16 
#define BOOK_SIZE 16

static void *allocstart;
static void *currentbreak;
static void *next;
void swrite(void *addr, char *s, uint64_t offset);

//16 bytes long
struct allocation {
    uint64_t free;
    uint64_t offset;
};

//return pointer to bookkeeping struct of first alloc 
//found with >= bytes available for user allocation
//return NULL if no free allocs are found
void *search(size_t bytes) {
    size_t allocsize = bytes + BOOK_SIZE;
    void *check = (char *)allocstart;
    struct allocation *checkalloc;
    while (check < currentbreak) {
        checkalloc = check;
        if (checkalloc->free == 1 && checkalloc->offset >= allocsize) {
            return check;
        } else if ((uint64_t *)checkalloc->offset == NULL) {
            void *nextallocspace = (char *)check + allocsize + BOOK_SIZE;
            if (currentbreak > nextallocspace) {
                return check;
            } else {
                next = (char *)check;
                return NULL;
            }
        }
        check = (char *)check + checkalloc->offset;
    }
    return NULL;
}

/*
void *search(size_t bytes) {
    size_t allocsize = bytes + BOOK_SIZE;
    void *check = (char *)allocstart;
    struct allocation *checkalloc;
    while (check < currentbreak) {
        checkalloc = check;
        if (checkalloc->free == 1 && checkalloc->offset >= allocsize) {
            if (checkalloc->offset > allocsize + (BOOK_SIZE*2)) {
                uint64_t ogoffset = checkalloc->offset;
                checkalloc->offset = allocsize;
                void *nextaddr = (char *)check + checkalloc->offset;
                struct allocation *nextalloc = nextaddr;
                nextalloc->free = 1;
                nextalloc->offset = ogoffset - checkalloc->offset;
            }
            checkalloc->free = 0;
            return check;
        } else if ((uint64_t *)checkalloc->offset == NULL) {
            void *nextallocspace = (char *)check + allocsize + BOOK_SIZE;
            if (currentbreak > nextallocspace) {
                checkalloc->free = 0;
                checkalloc->offset = (uint64_t)allocsize;
                
                void *nextallocaddr = (char *)check + checkalloc->offset;
                struct allocation *nextalloc = nextallocaddr;
                nextalloc->free = 1;
                nextalloc->offset = (uint64_t)NULL;

                return check;
            } else {
                next = (char *)check;
                return NULL;
            }
        }
        check = (char *)check + checkalloc->offset;
    }
    return NULL;
}
*/

void *malloc(size_t bytes){
    if (bytes == 0) {
        return NULL;
    }
    void *addr;
    if (bytes % BOUNDARY != 0) {
        bytes = bytes + (BOUNDARY - (bytes % BOUNDARY));
    }
    size_t minimumallocsize = bytes + (BOOK_SIZE*2);
    if (allocstart != NULL) {
        if ((addr = search(bytes)) != NULL) {
            struct allocation *alloc = addr;
            alloc->free = 0;
            if (alloc->offset != NULL) {
                void *nextallocaddr = (char *)addr + alloc->offset;
                struct allocation *nextalloc = nextallocaddr;
            } else {
                alloc->offset = bytes + BOOK_SIZE;

            }
            if (alloc->offset >= minimumallocsize + BOOK_SIZE) {
                
            }
            //swrite(addr , "m", NULL);
            return (void *)((char *)addr + BOOK_SIZE);
        }
    }
    if((addr = sbrk(minimumallocsize)) != (void *) -1) {
        if (allocstart == NULL){
            allocstart = (char *)addr;
        }
        if (next != NULL){
            addr = (char *)next;
            next = NULL;
        } 
        currentbreak = sbrk(0);

        //allocation to be returned
        struct allocation *alloc = (char *)addr;
        alloc->free = 0;
        alloc->offset = (bytes + BOOK_SIZE);

        //allocation following
        void *nextallocaddr = (char *)addr + alloc->offset;
        struct allocation *nextalloc = nextallocaddr;
        nextalloc->free = 1;
        nextalloc->offset = (uint64_t)NULL;

        //swrite(addr, "m", alloc->offset);
        return (void *)((char *)addr + BOOK_SIZE);
    } else {
        perror("sbrk: ");
        return NULL;
    }
}

//good
void free(void *ptr){
    if (ptr != NULL){
        //swrite(ptr, "f", NULL);
        struct allocation *alloc = (void *)((char *)ptr-BOOK_SIZE);
        alloc->free = 1;
    }
}

//good
void *calloc(size_t nmemb, size_t size){
    if (nmemb == 0 || size == 0){
        return NULL;
    }
    void *addr = malloc(nmemb*size);
    memset(addr, 0, nmemb*size);
    return addr;
}

int enlarge(void *ptr, size_t size) {
    void *check = ptr;
    struct allocation *checkalloc = check;
    //TODO maybe change below a lil bit
    size_t currentsize = checkalloc->offset - BOOK_SIZE;
    check = (char *)check + checkalloc->offset;
    if (check < currentbreak) {
        checkalloc = check;
    }
    while ((check < currentbreak) && (checkalloc->free == 1) &&
    (checkalloc->offset != (uint64_t)NULL)) {
        currentsize += (size_t)checkalloc->offset;
        if (currentsize >= size) {
            struct allocation *alloc = ptr;
            alloc->offset = currentsize+BOOK_SIZE;
            //uint64_t nextoffset = checkalloc->offset;
            return 1;
        }
        check = (char *)check + checkalloc->offset;
        checkalloc = check;
    }
    return 0;
}

void *realloc(void *ptr, size_t size) {
    if (ptr == NULL) {
        return malloc(size);
    }
    if (size == 0) {
        free(ptr);
    }
    if (enlarge(ptr, size) == 1) {
        return ptr;
    }
    //swrite(ptr, "r", size);
    free(ptr);
    return malloc(size);
}

/*
void *realloc(void *ptr, size_t size){
    if (ptr == NULL) {
        return malloc(size);
    }
    if (size == 0) {
        free(ptr);
    }
    //create starting address
    void *checkallocaddr = (char *)ptr - BOOK_SIZE;
    struct allocation *checkalloc = checkallocaddr;
    //get current size
    size_t currentsize = checkalloc->offset - BOOK_SIZE;
    checkallocaddr = (char *)checkallocaddr + checkalloc->offset;
    if (checkallocaddr < currentbreak) {
        checkalloc = checkallocaddr;
    } else {
        return malloc(size);
    }
    while ((checkallocaddr < currentbreak) && (checkalloc->free == 1)) {
        if (checkalloc->offset == NULL) {
            break;
        }
        currentsize += (size_t)checkalloc->offset;
        if (currentsize >= size){
            struct allocation *returnalloc = ptr;
            returnalloc->offset = (uint64_t)currentsize + BOOK_SIZE;
            swrite(ptr, 1, 3);
            return ptr;
        }
        checkallocaddr = (char *)checkallocaddr + checkalloc->offset;
        checkalloc = checkallocaddr;
    }
    return malloc(size);
}
*/
//based on code by dennis ritchie and brian kernighan
void itoa(uint64_t n, char s[]){
    uint64_t i = 0;
    do {
        s[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);
    char reverses[50];
    for (int x = 0; x < i; x++){
        reverses[x] = s[i-1-x];
    }
    for (int y = 0; y < i; y++){
        s[y] = reverses[y];
    }
    s[i] = '\0';
}

void swrite(void *addr, char *s, uint64_t offset) {
    int fd = open("memory.txt", O_CREAT|O_WRONLY|O_APPEND, 0777);
    char addrstring[50];
    char fullstring[60];
    strcpy(fullstring, s);
    itoa(addr, addrstring);
    strcat(fullstring, addrstring);
    //write(fd, s, strlen(s));
    write(fd, fullstring, strlen(fullstring));
    char fullstring2[60];
    char offsetstring[50];
    strcpy(fullstring2, "->");
    itoa(offset, offsetstring);
    strcat(fullstring2, offsetstring);
    strcat(fullstring2, "\n");
    write(fd, fullstring2, strlen(fullstring2));
    if (close(fd) == -1) {
        perror("close: ");
    }
}

/*
void swrite(void *addr, int type, int func) {
    int fd = open("memory.txt", O_CREAT|O_WRONLY|O_APPEND, 0777);
    void *allocaddress = (char *)addr - BOOK_SIZE;
    struct allocation *alloc = allocaddress;
    uint64_t address = (uint64_t)addr;
    uint64_t addr2 = (uint64_t)addr + alloc->offset;
    char addrstring[50];
    itoa(address , addrstring);
    char s2[50];
    itoa(alloc->offset, s2);
    char addr2string[50];
    itoa(addr2, addr2string);
    if (func == 1){
        write(fd, "m", 1);
    } else if (func == 2) {
        write(fd, "c", 1);
    } else if (func == 3) {
        write(fd, "r", 1);
    } else if (func == 4) {
        write(fd, "f", 1);
    }
    write(fd, addrstring, strlen(addrstring));
    write(fd, "->", 2);
    write(fd, addr2string, strlen(addr2string));
    write(fd, "->", 2);
    write(fd, s2, strlen(s2));
    write(fd, "\n", 1);
    if (close(fd) == -1){
        perror("close: ");
    }
}
*/
