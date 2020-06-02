#include <cstdlib>
#include <mutex>
#include <atomic>
#include <new>
#include <dlfcn.h>
#include <hyperloglog.hpp>
#include <tprintf.h>
#include <unistd.h>
#include "proc.hh"

/* 
    * HYPERLOGLOG MODIFICATIONS:
    
    * Change vector to buffer in cpp-HyperLogLog/include/hyperloglog.hpp
    * Replace vector init with memset in HLL constructor
    * Comment out every method but add and estimate
*/

class Data {
    public:
        Data() {
            void *map = get_proc_data();
            h = hll::HyperLogLog(16);
            num_allocs = new(map) std::atomic<int>(0);
            card = (double *) ((char *) map + sizeof(*num_allocs));
        }

        hll::HyperLogLog h;
        std::mutex mtx;
        std::atomic<int> *num_allocs;
        double *card;
};

inline static Data *get_data() {
    static char buf[sizeof(Data)];
    static Data *data = new(buf) Data;
    return data;
}

extern "C" void *xxmalloc(size_t size) {
    static void *(*real_malloc)(size_t) = nullptr;
    static bool is_dlsym = false;
    static Data *data = nullptr;
    void *ptr;

    if (is_dlsym) { // If isDlsym, then this is a recursive call to malloc
        return nullptr;
    }
    if (real_malloc == nullptr) { // If realMalloc is null, then we need to interpose malloc
        is_dlsym = true;
        real_malloc = (void *(*)(size_t)) dlsym(RTLD_NEXT, "malloc");
        is_dlsym = false;
        if (real_malloc == nullptr) { // Make sure dlsym worked
            tprintf::tprintf("libentroprise: ERROR: cannot dlsym malloc\n");
            exit(EXIT_FAILURE);
        }
    }

    data = get_data(); // Fetch rest of data
    ptr = real_malloc(size);
    data->mtx.lock();
    data->h.add((char *) &ptr, sizeof(void *));
    // *(data->card) = data->h.estimate();
    data->mtx.unlock();
    *(data->num_allocs) = *(data->num_allocs) + 1;
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
