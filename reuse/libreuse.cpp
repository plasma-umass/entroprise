#include <atomic>
#include <mutex>
#include <new>
#include <cstdlib>
#include <dlfcn.h>
#include "tprintf.h"
#include "proc.hh"
#include "fatal.hh"

class Data {
    public:
        Data() : num_allocs(0), num_live(0) {};

        std::atomic<int> num_allocs;
        std::atomic<int> num_live;
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
    int local_num_allocs, local_num_live;

    if (real_malloc == nullptr) { // If real_malloc is null, then we need to interpose malloc
        if (is_dlsym) { // If is_dlsym, then this is a recursive call to malloc through dlsym
            return nullptr;
        }
        is_dlsym = true;
        real_malloc = (void *(*)(size_t)) dlsym(RTLD_NEXT, "malloc");
        is_dlsym = false;
        if (real_malloc == nullptr) { // Make sure dlsym worked
            fatal((char *) "cannot dlsym malloc\n");
        }
    }

    data = get_data(); // Fetch rest of data
    ptr = real_malloc(size);
    data->num_allocs.fetch_add(1);
    data->num_live.fetch_add(1);
    local_num_allocs = data->num_allocs.load();
    local_num_live = data->num_live.load();
    tprintf::tprintf("num_allocs = @, num_live = @, reuse = @\n", local_num_allocs, local_num_live, (double) local_num_allocs / local_num_live);
    return ptr;
}

extern "C" __attribute__((always_inline)) void xxfree(void *ptr) {
    static void (*real_free)(void *) = nullptr;
    static Data *data = nullptr;
    if (real_free == nullptr) {
        real_free = (void (*)(void *)) dlsym(RTLD_NEXT, "free");
    }
    data = get_data();
    data->num_live.fetch_add(-1);
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
