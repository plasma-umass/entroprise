#include <atomic>
#include <mutex>
#include <new>
#include <cstdlib>
#include <cstdio>
#include <unordered_map>
#include <heaplayers>
#include <dlfcn.h>
#include <pthread.h>
#include "hyperloglog.hpp"
#include "tprintf.h"
#include "proc.hh"
#include "fatal.hh"
#ifdef ENTROPRISE_BACKTRACE
#include <execinfo.h>
#include <signal.h>
#endif

/* 
    * HYPERLOGLOG MODIFICATIONS:
    (https://github.com/hideo55/cpp-HyperLogLog)
    
    * Change vector to buffer in cpp-HyperLogLog/include/hyperloglog.hpp
    * Replace vector init with memset in HLL constructor
    * Comment out every method but add and estimate
    * Add a constructor that takes a char * and does nothing

    * MAKE SURE .addrs.bin IS LARGE ENOUGH
*/

#ifdef ENTROPRISE_BACKTRACE

void handler(int sig) {
    void *buf[20];
    int backtrace_size = backtrace(buf, 20);
    backtrace_symbols_fd(buf, backtrace_size, STDERR_FILENO);
    exit(EXIT_FAILURE);
}

#endif

class ThreadData {
    public:
        ThreadData(int tid) { 
            char fname[100];
            const unsigned int INIT_ADDRS = 8192, INIT_SIZE = sizeof(int) + sizeof(hll::HyperLogLog) + sizeof(void *) * INIT_ADDRS;
            snprintf(fname, 100, "%d.threads.bin", tid);
            create_thread_data(&fd, &map, fname, INIT_SIZE);
            num_allocs = (int *) map;
            *num_allocs = 0;
            addrs_cap = INIT_ADDRS;
            h = new(num_allocs + 1) hll::HyperLogLog;
            addrs = (void **) (h + 1);
            this->tid = tid;
        }

        void *map;
        int *num_allocs, capacity, fd, addrs_cap;
        hll::HyperLogLog *h;
        void **addrs;
        int tid;
};

// Custom HeapLayers allocator that unordered_map allocates from
template <typename T>
class MyAllocator : public STLAllocator<T, FreelistHeap<BumpAlloc<4096, MmapHeap>>> {};
// class MyAllocator : public STLAllocator<T, LockedHeap<std::mutex, FreelistHeap<BumpAlloc<4096, MmapHeap>>>> {};

class GlobalData {
    public:
        // GlobalData() : num_threads(0) {}
        GlobalData() {
            num_threads = 0;
        }

        std::unordered_map<pthread_t, ThreadData, std::hash<pthread_t>, equal_to<pthread_t>, MyAllocator<pair<pthread_t, ThreadData>>> m;
        // std::atomic<int> num_threads;
        int num_threads;
};

static __attribute__((always_inline)) GlobalData *get_global_data() {
    static char buf[sizeof(GlobalData)];
    static GlobalData *data = new(buf) GlobalData;
    return data;
}

extern "C" __attribute__((always_inline)) void *xxmalloc(size_t size) {
    static void *(*real_malloc)(size_t) = nullptr;
    static bool is_dlsym = false;
    static GlobalData *gdata = nullptr;
    void *ptr;
    static std::mutex mtx;
    pthread_t tid;
    ThreadData *tdata;

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

        #ifdef ENTROPRISE_BACKTRACE
        if (signal(SIGSEGV, handler) == SIG_ERR) {
            fatal((char *) "could not signal SIGSEGV\n");
        }
        if (signal(SIGINT, handler) == SIG_ERR) {
            fatal((char *) "could not signal SIGINT\n");
        }
        #endif
    }

    gdata = get_global_data(); // Fetch rest of data
    ptr = real_malloc(size);
    tid = pthread_self();
    mtx.lock();
    if (gdata->m.find(tid) == gdata->m.end()) {
        // int n = gdata->num_threads.fetch_add(1);
        int n = gdata->num_threads++;
        gdata->m.emplace(tid, n);
    }
    tdata = &(gdata->m.find(tid)->second); // Fetch this thread's data
    mtx.unlock();
    tdata->h->add((char *) &ptr, sizeof(void *)); // Add address to HyperLogLog
    if (*(tdata->num_allocs) == tdata->addrs_cap) {
        int old_sz = sizeof(int) + sizeof(hll::HyperLogLog) + sizeof(void *) * tdata->addrs_cap;
        int new_sz = sizeof(int) + sizeof(hll::HyperLogLog) + sizeof(void *) * tdata->addrs_cap * 2;
        tdata->map = extend_data(tdata->fd, tdata->map, old_sz, new_sz);
        tdata->num_allocs = (int *) tdata->map;
        tdata->h = new(tdata->num_allocs + 1) hll::HyperLogLog((char *) nullptr);
        tdata->addrs = (void **) (tdata->h + 1);
        tdata->addrs_cap *= 2;
    }
    tdata->addrs[*(tdata->num_allocs)] = ptr; // Add address to list of addresses
    *(tdata->num_allocs) = *(tdata->num_allocs) + 1; // Increment number of allocations
    #ifdef ENTROPRISE_DEBUG
    static std::mutex write_mtx;
    write_mtx.lock();
    tprintf::tprintf("Thread @; @ Allocations; malloc(@) = @\n", tdata->tid, *(tdata->num_allocs), size, ptr);
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
