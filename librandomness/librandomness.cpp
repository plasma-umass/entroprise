#include <atomic>
#include <mutex>
#include <new>
#include <dlfcn.h>
#include <tprintf.h>
#include "proc.hh"
#include "fatal.hh"

class Data {
    public:
        Data() {
            void *map = get_proc_data();
            num_allocs = new(map) std::atomic<int>(0);
            addrs = (void **) (num_allocs + 1); // Point addrs to address immediately proceeding numAllocs
        }

        std::atomic<int> *num_allocs;
        void **addrs;
};

static __attribute__((always_inline)) Data *get_data() {
    static char buf[sizeof(Data)];
    static Data *data = new(buf) Data;
    return data;
}

extern "C" __attribute__((always_inline)) void *xxmalloc(size_t size) {
    static void *(*real_malloc)(size_t) = nullptr;
    static bool is_dlsym = false;
    static Data *data;
    void *ptr;
    int next;

    if (real_malloc == nullptr) {
        if (is_dlsym) { // For recursive call to malloc through dlsym -> calloc -> malloc
            return nullptr;
        }
        is_dlsym = true;
        real_malloc = (void *(*)(size_t)) dlsym(RTLD_NEXT, "malloc");
        is_dlsym = false;
        if (real_malloc == nullptr) { // Make sure dlsym worked
            fatal((char *) "cannot dlsym malloc\n");
        }
    }

    data = get_data();
    ptr = real_malloc(size);
    next = data->num_allocs->fetch_add(1); // Increment atomically
    data->addrs[next] = ptr; // Add address atomically
    #ifdef RANDOMNESS_DEBUG
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
