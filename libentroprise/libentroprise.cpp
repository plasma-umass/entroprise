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

void *(*realMalloc)(size_t) = NULL;
std::mutex *mtx = new std::mutex();
hll::HyperLogLog *addrs = new hll::HyperLogLog(6); // TODO: Adjust value?
int numAllocs = 0, nextIndex = 1, fd;
double cardinality, entropy, maxEntropy, percentage;

void *malloc(size_t size) {
    void *ptr;
    if (realMalloc == NULL) { // First malloc
        realMalloc = (void *(*)(size_t)) dlsym(RTLD_NEXT, "malloc");
        if (realMalloc == NULL) {
            return NULL;
        }
        fd = creat("entroprise-results.out", S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
        if (fd == -1) {
            write(STDERR_FILENO, "libentroprise: Cannot create out file\n", 38);
        }
    }
    ptr = realMalloc(size);
    if (mtx == NULL || addrs == NULL) { // Call to malloc is either for mtx or addrs
        return ptr;
    }
    mtx->lock();
    addrs->add((char *) &ptr, sizeof(void *));
    numAllocs++;
    if (numAllocs == nextIndex) { // Print data every power of two
        nextIndex <<= 1;
        cardinality = addrs->estimate();
        entropy = log(cardinality) / log(2.0);
        maxEntropy = log(numAllocs) / log(2.0);
        if (maxEntropy == 0) {
            percentage = 100;
        } else {
            percentage = entropy * 100.0 / maxEntropy;
        }
        tprintf::tprintf(
            "Number of Allocations: @\n"
            "Number of Unique Addresses: @\n"
            "Calculated Entropy: @\n"
            "Maximum Entropy: @\n"
            "Percentage of Maximum: @%\n\n",
            numAllocs, cardinality, entropy, maxEntropy, percentage
        );
    }
    mtx->unlock();
    return ptr;
}
