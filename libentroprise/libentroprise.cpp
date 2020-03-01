#include <cstdlib>
#include <cmath>
#include <mutex>
#include <atomic>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <hyperloglog.hpp>
#include <murmur3.h>
#include <tprintf.h>
// #define ENTROPRISE_DEBUG
#ifdef ENTROPRISE_DEBUG
    #define PDEBUG(fmt, args...) tprintf::tprintf(fmt, ## args)
#else
    #define PDEBUG(fmt, args...) 
#endif

hll::HyperLogLog *addrs = new hll::HyperLogLog(6);
std::atomic<bool> isFirstMalloc(true);
int numAllocs = 0, nextIndex = 1, fd;
void *(*realMalloc)(size_t);
std::mutex hppLock, cmpLock;

inline void writeEntropy() {
    double cardinality, entropy, percentage, max;
    max = log(numAllocs) / log(2.0);
    cmpLock.unlock();
    hppLock.lock();
    cardinality = addrs->estimate();
    hppLock.unlock();
    max = log(numAllocs) / log(2.0);
    entropy = log(cardinality) / log(2.0);
    if (max == 0) {
        percentage = 100;
    } else {
        percentage = entropy * 100.0 / max;
    }
    tprintf::tprintf(
        "Number of Allocations: @\n"
        "Number of Unique Addresses: @\n"
        "Calculated Entropy: @\n"
        "Maximum Entropy: @\n"
        "Percentage of Maximum: @%\n\n",
        numAllocs, cardinality, entropy, max, percentage
    );
}

void *malloc(size_t size) {
    void *ptr;
    int localNumAllocs, localNextIndex;
    if (std::atomic_exchange(&isFirstMalloc, false)) {
        realMalloc = (void *(*)(size_t)) dlsym(RTLD_NEXT, "malloc");
        fd = creat("entroprise-results.out", S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
        PDEBUG("Should only print once\n");
    }
    ptr = realMalloc(size);
    if (addrs == NULL) {
        return ptr;
    }
    PDEBUG("Address = @, Size = @\n", ptr, size);
    hppLock.lock();
    addrs->add((char *) &ptr, sizeof(void *));
    hppLock.unlock();
    cmpLock.lock();
    numAllocs++;
    if (numAllocs == nextIndex) {
        writeEntropy();
        cmpLock.lock();
        nextIndex <<= 1;
    }
    cmpLock.unlock();
    return ptr;
}
