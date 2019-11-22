#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#define BOUNDARY 16

static void *start;
static void *currentbrk;
static void *next;

int testnext(size_t size);
void *search(size_t size);

//16 bytes long
struct allocation {
    uint64_t free;
    uint64_t offset;
};

int testnext(size_t size) {
    //TODO check if this works for equals
    if ((void *)((char *)next + size) <= currentbrk) {
        return 1;
    } else {
        return 0;
    }
}

void *search(size_t size) {
    void *check = (char *)start;
    struct allocation *checkalloc;
    while (check < currentbrk && check < next) {
        checkalloc = check;
        if (checkalloc->free == 1 && checkalloc->offset >= size) {
            return check;
        }
        check = (char *)check + checkalloc->offset;
    }
    return NULL;
}

void free(void *ptr) {
    if (ptr != NULL) {
        struct allocation *alloc = (void *)((char *)ptr - sizeof(struct allocation));
        alloc->free = 1;
    }
}

void *malloc(size_t size) {
    void *addr;
    size_t fullsize = size + sizeof(struct allocation);
    if (fullsize % BOUNDARY != 0) {
        fullsize = fullsize + (BOUNDARY - (fullsize % BOUNDARY));
    }
    //if malloc has been called before
    if (start != NULL) {
        //if search finds a suitable freed alloc, return it
        if ((addr = search(fullsize)) != NULL) {
            struct allocation *alloc = addr;
            alloc->free = 0;
            return (char *)addr + sizeof(struct allocation);
        }
        if (testnext(fullsize)) {
            struct allocation *alloc = next;
            alloc->free = 0;
            alloc->offset = fullsize;
        }
    }
    if ((addr = sbrk(fullsize)) != (void *) -1) {
        if (start == NULL) {
            start = (char *)addr;
        }
        if (next != NULL) {
            addr = (char *)next;
        } 
        currentbrk = sbrk(0);
        struct allocation *alloc = addr;
        alloc->free = 0;
        alloc->offset = fullsize;
        next = (char *)addr + alloc->offset;
        return (char *)addr + sizeof(struct allocation);
   } else {
        exit(1);
   }
}

void *calloc(size_t nmemb, size_t size) {
    void *addr = malloc(nmemb*size);
    memset(addr, 0, nmemb*size);
    return addr;
}

void *realloc(void *ptr, size_t size) {
    if (ptr == NULL) {
        return malloc(size);
    }
    if (size == 0) {
        free(ptr);
        return NULL;
    }
    free(ptr);
    return malloc(size);
}
