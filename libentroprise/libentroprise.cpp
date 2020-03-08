#include <cstdlib>
#include <cmath>
#include <mutex>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <hyperloglog.hpp>
#include <murmur3.h>
#include <tprintf.h>

hll::HyperLogLog *addrs = new hll::HyperLogLog(6);
bool isFirstMalloc = true;
int numAllocs = 0, nextIndex = 1, fd = -1;
void *(*realMalloc)(size_t);
std::mutex *hppLock = new std::mutex(), *cmpLock = new std::mutex(), *printLock = new std::mutex();

inline void writeEntropy() {
    double cardinality, entropy, percentage, max;
    max = log(numAllocs) / log(2.0);
    cmpLock->unlock();
    hppLock->lock();
    cardinality = addrs->estimate();
    hppLock->unlock();
    entropy = log(cardinality) / log(2.0);
    if (max == 0) {
        percentage = 100;
    } else {
        percentage = entropy * 100.0 / max;
    }
    printLock->lock();
    tprintf::tprintf(
        "Number of Allocations: @\n"
        "Number of Unique Addresses: @\n"
        "Calculated Entropy: @\n"
        "Maximum Entropy: @\n"
        "Percentage of Maximum: @%\n\n",
        numAllocs, cardinality, entropy, max, percentage
    );
    printLock->unlock();
}

void *malloc(size_t size) {
    void *ptr;
    if (isFirstMalloc) {
        realMalloc = (void *(*)(size_t)) dlsym(RTLD_NEXT, "malloc");
        if (realMalloc == NULL) {
            return NULL;
        }
        fd = creat("entroprise-results.out", S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
        if (fd == -1) {
            write(STDERR_FILENO, "ERROR: Cannot create out file\n", 30);
        }
        isFirstMalloc = false;
    }
    ptr = realMalloc(size);
    if (addrs == NULL || hppLock == NULL || cmpLock == NULL || printLock == NULL) {
        return ptr;
    }
    PDEBUG("Address = @, Size = @\n", ptr, size);
    hppLock->lock();
    addrs->add((char *) &ptr, sizeof(void *));
    hppLock->unlock();
    cmpLock->lock();
    numAllocs++;
    if (numAllocs == nextIndex) {
        nextIndex <<= 1;
        writeEntropy(); // Releases cmpLock
    } else {
        cmpLock->unlock();
    }
    return ptr;
}
