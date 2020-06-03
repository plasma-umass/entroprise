#include <mutex>
#include <atomic>
#include <new>
#include <cstdlib>
#include <dlfcn.h>
#include "hyperloglog.hpp"
#include "tprintf.h"
#include "proc.hh"
#include "fatal.hh"

/* 
    * HYPERLOGLOG MODIFICATIONS:
    
    * Change vector to buffer in cpp-HyperLogLog/include/hyperloglog.hpp
    * Replace vector init with memset in HLL constructor
    * Comment out every method but add and estimate
    * Add a constructor that takes a char * and does nothing
*/

class Data {
    public:
        Data() {
            void *map = get_proc_data();
            num_allocs = new(map) std::atomic<int>(0);
            h = new(num_allocs + 1) hll::HyperLogLog;
        }

        std::atomic<int> *num_allocs;
        hll::HyperLogLog *h;
        std::mutex mtx;
};

static __attribute__((always_inline)) Data *get_data() {
    static char buf[sizeof(Data)];
    static Data *data = new(buf) Data;
    return data;
}

extern "C" __attribute__((always_inline)) void *xxmalloc(size_t size) {
    static void *(*real_malloc)(size_t) = nullptr;
    static bool is_dlsym = false;
    static Data *data = nullptr;
    void *ptr;

    if (real_malloc == nullptr) { // If real_malloc is null, then we need to interpose malloc
        if (is_dlsym) { // If is_dlsym, then this is a recursive call to malloc through dlsym
            return nullptr;
        }
        is_dlsym = true;
        real_malloc = (void *(*)(size_t)) dlsym(RTLD_NEXT, "malloc");
        is_dlsym = false;
        if (real_malloc == nullptr) { // Make sure dlsym worked
            fatal("libentroprise: ERROR: cannot dlsym malloc\n");
        }
    }

    data = get_data(); // Fetch rest of data
    ptr = real_malloc(size);
    data->mtx.lock();
    data->h->add((char *) &ptr, sizeof(void *)); // Add address to HyperLogLog
    data->mtx.unlock();
    data->num_allocs->fetch_add(1); // Increment atomically
    #ifdef ENTROPRISE_DEBUG
    static std::mutex write_mtx;
    write_mtx.lock();
    tprintf::tprintf("@: malloc(@) = @\n", data->num_allocs->load(), size, ptr);
    write_mtx.unlock();
    #endif
    return ptr;
}

extern "C" __attribute__((always_inline)) void xxfree(void *ptr) {
    static void (*real_free)(void *) = nullptr;
    if (real_free == nullptr) {
        real_free = (void (*)(void *)) dlsym(RTLD_NEXT, "free");
    }
    real_free(ptr);
}

extern "C" __attribute__((always_inline)) size_t xxmalloc_usable_size(void *ptr) {
    static size_t (*real_malloc_usable_size)(void *) = nullptr;
    if (real_malloc_usable_size == nullptr) {
        real_malloc_usable_size = (size_t(*)(void *)) dlsym(RTLD_NEXT, "malloc_usable_size");
    }
    return real_malloc_usable_size(ptr);
}

extern "C" __attribute__((always_inline)) void xxmalloc_lock(void) {
    return;
}

extern "C" __attribute__((always_inline)) void xxmalloc_unlock(void) {
    return;
}
