#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <dlfcn.h>

extern "C" void *xxmalloc(size_t size) {
    static void *(*realMalloc)(size_t) = nullptr;
    static bool hasDlsym = false;
    static int fd, index = 0, *numAllocs;
    static void **addrs;
    static struct stat statBuf;
    void *ptr;
    if (realMalloc == nullptr) {
        if (hasDlsym) {
            return nullptr;
        }
        hasDlsym = true;
        realMalloc = (void *(*)(size_t)) dlsym(RTLD_NEXT, "malloc");
        hasDlsym = false;
        fd = open("addrs.bin", O_RDWR);
        if (fd == -1) {
            exit(EXIT_FAILURE);
        }
        if (fstat(fd, &statBuf) == -1) {
            exit(EXIT_FAILURE);
        }
        numAllocs = (int *) mmap(NULL, statBuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (numAllocs == MAP_FAILED) {
            exit(EXIT_FAILURE);
        }
        if (madvise(numAllocs, statBuf.st_size, MADV_SEQUENTIAL) == -1) {
            exit(EXIT_FAILURE);
        }
        *numAllocs = 0;
        addrs = (void **) (numAllocs + 1);
    }
    ptr = realMalloc(size);
    *numAllocs = *numAllocs + 1;
    addrs[index++] = ptr;
    return ptr;
}

extern "C" void xxfree(void *ptr) {
    static void (*real_free)(void *) = nullptr;
    if (real_free == nullptr) {
        real_free = (void (*)(void *)) dlsym(RTLD_NEXT, "free");
    }
    real_free(ptr);
}

extern "C" size_t xxmalloc_usable_size(void *ptr) {
    static size_t (*real_malloc_usable_size)(void *) = nullptr;
    if (real_malloc_usable_size == nullptr) {
        real_malloc_usable_size = (size_t(*)(void *)) dlsym(RTLD_NEXT, "malloc_usable_size");
    }
    return real_malloc_usable_size(ptr);
}

extern "C" void xxmalloc_lock(void) {
    return;
}

extern "C" void xxmalloc_unlock(void) {
    return;
}
