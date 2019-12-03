#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <unistd.h>

#define ALIGNMENT 16

static void *start;
static void *currentbrk;
static void *next;

static int testnext(size_t size);
static void split(void *ptr, size_t size);
static void *search(size_t size);
static void printuint(void *val, char *s);

//16 bytes long
struct allocation {
    uint64_t free;
    uint64_t offset;
};

//non-user function
//returns 1 if there is enough empty space
//before the brea to accomodate size
static int testnext(size_t size) {
    if ((void *)((char *)next + size) < currentbrk) {
        return 1;
    } else {
        return 0;
    }
}

//non-user function
//returns pointer to alloc with offset greater than
//or equal to size
static void *search(size_t size) {
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

//non-user function
//for program use, assumes pointer is to a correct alloc
//splits allocation into two if there is enough space for
//a second allocation after an alloc of size is made
static void split(void *ptr, size_t size) {
    if (size % ALIGNMENT != 0) {
        size = size + (ALIGNMENT - (size % ALIGNMENT));
    }
    struct allocation *splitalloc = ptr;
    size_t minimumsize = sizeof(struct allocation) + ALIGNMENT;
    if (splitalloc->offset >= size + minimumsize) {
        uint64_t oldoffset = splitalloc->offset;
        void *newaddr = (char *)ptr + size;
        struct allocation *newalloc = newaddr;
        splitalloc->offset = size;
        newalloc->free = 1;
        newalloc->offset = oldoffset - size;
    }
}

//user function
//frees alloc corresponding to user provided pointer
void free(void *ptr) {
    if (ptr != NULL) {
        void *allocaddr = (char *)ptr - sizeof(struct allocation);
        struct allocation *alloc = allocaddr;
        alloc->free = 1;
    }
}

//user function
void *malloc(size_t size) {
    if (size == 0) {
        return NULL;
    }
    void *addr;
    size_t fullsize = size + sizeof(struct allocation);
    //rounded up so it is aligned on the boundary
    if (fullsize % ALIGNMENT != 0) {
        fullsize = fullsize + (ALIGNMENT - (fullsize % ALIGNMENT));
    }
    if (start != NULL) {
        if ((addr = search(fullsize)) != NULL) {
            split(addr, fullsize);
            struct allocation *alloc = addr;
            alloc->free = 0;
            return (char *)addr + sizeof(struct allocation);
        }
        if (testnext(fullsize)) {
            addr = (char *)next;
            struct allocation *alloc = addr;
            alloc->free = 0;
            alloc->offset = (uint64_t)fullsize;
            next = (char *)next + alloc->offset;
            return (char *)addr + sizeof(struct allocation);
        }
    }
    size_t sbrksize;
    if (fullsize < 4096) {
        sbrksize = 4096;
    } else {
        sbrksize = fullsize + (4096 - (fullsize % 4096));
        sbrksize = sbrksize + 4096;
    }
    if ((addr = sbrk(sbrksize)) != (void *) -1) {
        struct allocation *alloc;
        if (start == NULL) {
            start = (char *)addr;
        }
        if (next != NULL) {
            addr = (char *)next;
        }
        currentbrk = sbrk(0);
        alloc = addr;
        alloc->free = 0;
        alloc->offset = (uint64_t)fullsize;
        next = (char *)addr + alloc->offset;
        return (char *)addr + sizeof(struct allocation);
    } else {
        exit(1);
    }
}

void *calloc(size_t nmemb, size_t size) {
    if ((nmemb == 0) || (size == 0)) {
        return NULL;
    }
    void *addr = malloc(nmemb*size);
    memset(addr, 0, nmemb*size);
    return addr;
}

void *realloc(void *ptr, size_t size) {
    if ((ptr == NULL) && (size == 0)) {
        return NULL;
    } else if (ptr == NULL) {
        return malloc(size);
    } else if (size == 0) {
        free(ptr);
    }
    void *allocpointer = (char *)ptr - sizeof(struct allocation);
    struct allocation *alloc = allocpointer;
    struct allocation *useralloc = allocpointer;
    size_t fullsize = size + sizeof(struct allocation);
    if (fullsize % ALIGNMENT != 0) {
        fullsize = fullsize + (ALIGNMENT - (fullsize % ALIGNMENT));
    }
    size_t currentsize = 0;
    alloc->free = 1;
    while ((allocpointer < currentbrk) && (alloc->free == 1)) {
        currentsize += alloc->offset;
        if (currentsize >= fullsize) {
            useralloc->free = 0;
            useralloc->offset = currentsize;
            return ptr;
        }
        allocpointer += alloc->offset;
        alloc = allocpointer;
    }
    void *addr = malloc(size);
    free(ptr);
    memcpy(addr, ptr, useralloc->offset);
    return addr;
}

//based on code by dennis ritchie and brian kernighan
void itoa(uint64_t n, char s[]) {
    uint64_t i = 0;
    do {
        s[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);
    char reverses[strlen(s)];
    for (int j = 0; j < i; j++) {
        reverses[j] = s[i-1-j];
    }
    for (int k = 0; k < i; k++) {
        s[k] = reverses[k];
    }
    s[i] = '\0';
}
