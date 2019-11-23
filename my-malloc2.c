#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <unistd.h>

#define BOUNDARY 16

static void *start;
static void *currentbrk;
static void *next;

int testnext(size_t size);
void *search(size_t size);
void printuint(void *val, char *s);

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

//for program use, assumes pointer is to a correct alloc
void split(void *ptr, size_t size) {
    struct allocation *checkalloc = ptr;
    size_t minimumsize = sizeof(struct allocation) + BOUNDARY;
    if (checkalloc->offset >= size + minimumsize) {
        uint64_t oldoffset = checkalloc->offset;
        checkalloc->offset = size;
        void *newaddr = (char *)ptr + checkalloc->offset;
        struct allocation *newalloc = newaddr;
        newalloc->free = 1;
        newalloc->offset = oldoffset - checkalloc->offset;
    }
}

void free(void *ptr) {
    if (ptr != NULL) {
        struct allocation *alloc = (void *)((char *)ptr - sizeof(struct allocation));
        alloc->free = 1;
    }

}

int enlarge(void *ptr, size_t size) {
    size_t currentsize = size;
    void *check = (void *)ptr;
    struct allocation *checkalloc = check;
    check = (char *)check + checkalloc->offset;
    checkalloc = check;
    while ((check < currentbrk) && (checkalloc->free == 1)) {
        currentsize += checkalloc->offset;
        if (currentsize >= size) {
            return 1;
        }
        check = (char *)check + checkalloc->offset;
        checkalloc = check;
    }
    return 0;
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
            split(addr, fullsize);
            struct allocation *alloc = addr;
            alloc->free = 0;
            printuint(fullsize, "m1");
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
        printuint(fullsize, "m");
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
    printuint(nmemb*size, "c");
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
    size_t minimumsize = size + sizeof(struct allocation);
    if (enlarge(ptr, minimumsize)) {
        split(ptr, minimumsize);
        return ptr;
    }
    free(ptr);
    return malloc(size);
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

void printuint(void *val, char *s) {
    int fd = open("memory", O_CREAT|O_WRONLY|O_APPEND, 0777);
    char valstring[50];
    char printstring[55];
    itoa((uint64_t)val, valstring);
    strcpy(printstring, s);
    strcat(printstring, valstring);
    strcat(printstring, "\n");
    write(fd, printstring, strlen(printstring));
    if (close(fd) == -1) {
        perror("close: ");
        exit(1);
    }
}
