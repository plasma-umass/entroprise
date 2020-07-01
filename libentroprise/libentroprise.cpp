#include <mutex>
#include <new>
#include <unordered_map>
#include <dlfcn.h>
#include <pthread.h>
#include <heaplayers>
#include "hyperloglog.hpp"
#include "proc.hh"
#include "fatal.hh"

#ifdef ENTROPRISE_BACKTRACE
#include <execinfo.h>
#include <signal.h>
#endif

#ifdef ENTROPRISE_DEBUG
#include "tprintf.h"
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

// If the ENTROPRISE_BACKTRACE macro is defined, then a stack backtrace will be printed out if SIGSEGV or SIGINIT signals are received
void handler(int sig) {
    void *buf[20];
    int backtrace_size = backtrace(buf, 20);
    backtrace_symbols_fd(buf, backtrace_size, STDERR_FILENO);
    exit(EXIT_FAILURE);
}

#endif

// Each thread is given its own ThreadData object to store its per-thread data
class ThreadData {
    public:
        ThreadData(int tid) { 
            char fname[256];
            const unsigned int INIT_ADDRS = 8192, INIT_SIZE = sizeof(int) + sizeof(hll::HyperLogLog) + sizeof(void *) * INIT_ADDRS;
            snprintf(fname, 256, THREAD_FILE_PREFIX "%d" THREAD_FILE_POSTFIX, tid);
            create_thread_data(&fd, &map, fname, INIT_SIZE);
            num_allocs = (int *) map;
            *num_allocs = 0;
            h = new(num_allocs + 1) hll::HyperLogLog;
            addrs = (void **) (h + 1);
            addrs_cap = INIT_ADDRS;
            this->tid = tid;
        }

        int *num_allocs; // Data stored in the mapping to fd
        hll::HyperLogLog *h;
        void **addrs;

        void *map; // Data stored in ThreadData objects
        int fd, tid, addrs_cap;
};

// Custom HeapLayers allocator that unordered_map allocates from 
// No need for LockedHeap because access to the unordered_map is controlled by a mutex
template <typename T>
class MyAllocator : public STLAllocator<T, FreelistHeap<BumpAlloc<4096, MmapHeap>>> {};

// Every thread shares one GlobalData object
class GlobalData {
    public:
        GlobalData() {
            num_threads = 0;
        }

        // unordered_map that maps thread IDs (pthread_t) to ThreadData objects and uses the HeapLayers generated allocator
        std::unordered_map<pthread_t, ThreadData, std::hash<pthread_t>, equal_to<pthread_t>, MyAllocator<pair<pthread_t, ThreadData>>> m;
        int num_threads;
        std::mutex mtx; // Anytime a thread access or modifies data in GlobalData, it must acquire GlobalData's mutex beforehand
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
    pthread_t tid;
    ThreadData *tdata;
    int old_size, new_size;

    if (real_malloc == nullptr) { // If real_malloc is null, then we need to interpose malloc
        if (is_dlsym) { // If is_dlsym, then this is a recursive call to malloc through dlsym
            return nullptr;
        }
        is_dlsym = true;
        real_malloc = (void *(*)(size_t)) dlsym(RTLD_NEXT, "malloc"); // CAREFUL - dlsym calls calloc
        is_dlsym = false;
        if (real_malloc == nullptr) { // Verify that dlsym was successful
            fatal((char *) "cannot dlsym malloc\n");
        }

        // If the ENTROPRISE_BACKTRACE macro is defined, then create handles for SIGSEGV and SIGINT
        #ifdef ENTROPRISE_BACKTRACE
        if (signal(SIGSEGV, handler) == SIG_ERR) {
            fatal((char *) "could not signal SIGSEGV\n");
        }
        if (signal(SIGINT, handler) == SIG_ERR) {
            fatal((char *) "could not signal SIGINT\n");
        }
        #endif
    }

    gdata = get_global_data(); // Fetch global data
    ptr = real_malloc(size);
    tid = pthread_self();
    gdata->mtx.lock();
    if (gdata->m.find(tid) == gdata->m.end()) { // If the unordered_map does not contain this thread ID...
        int n = gdata->num_threads++; // Then increase the number of threads
        gdata->m.emplace(tid, n); // And insert it
    }
    tdata = &(gdata->m.find(tid)->second); // Fetch this thread's data
    gdata->mtx.unlock();
    tdata->h->add((char *) &ptr, sizeof(void *)); // Add address to HyperLogLog
    if (*(tdata->num_allocs) == tdata->addrs_cap) { // If this thread cannot store any more addresses...
        int old_size = sizeof(int) + sizeof(hll::HyperLogLog) + sizeof(void *) * tdata->addrs_cap;
        int new_size = sizeof(int) + sizeof(hll::HyperLogLog) + sizeof(void *) * tdata->addrs_cap * 2; // Then increase the capacity by a factor of two
        tdata->map = extend_data(tdata->fd, tdata->map, old_size, new_size); // Extend the data region
        tdata->num_allocs = (int *) tdata->map; // And change the corresponding pointers
        tdata->h = new(tdata->num_allocs + 1) hll::HyperLogLog((char *) nullptr);
        tdata->addrs = (void **) (tdata->h + 1);
        tdata->addrs_cap *= 2;
    }
    tdata->addrs[*(tdata->num_allocs)] = ptr; // Add address to list of addresses
    *(tdata->num_allocs) = *(tdata->num_allocs) + 1; // Increment number of allocations
    // If the ENTROPRISE_DEBUG macro is defined, then for every call to malloc, print out the thread ID, number of allocations, and address generated by malloc
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
