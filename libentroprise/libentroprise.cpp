#include <cstdlib>
#include <cmath>
#include <mutex>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <hyperloglog.hpp>
#include <tprintf.h>
#include <unistd.h>
#include <string.h>

/* 
    * HYPERLOGLOG MODIFICATIONS:
    
    * Change vector to buffer in cpp-HyperLogLog/include/hyperloglog.hpp
    * Replace vector init with memset in HLL constructor
    * Comment out every method but add and estimate
*/

class Data {
    public:
        Data() {
            addrs = hll::HyperLogLog(16);
            numAllocs = 0;
            nextIndex = 1;
        }

        std::mutex dataMtx, writeMtx;
        hll::HyperLogLog addrs;
        int numAllocs, nextIndex;
};

inline static Data *getData() {
    static char buf[sizeof(Data)];
    static Data *data = new (buf) Data;
    return data;
}

void *malloc(size_t size) {
    static void *(*realMalloc)(size_t) = nullptr;
    static bool isDlsym = false;
    static char *err = (char *) "libentroprise: ERROR: cannot dlsym malloc\n";
    static Data *data = nullptr;
    int localNumAllocs;
    double cardinality, entropy, maxEntropy, percentage;
    void *ptr;

    if (isDlsym) { // If isDlsym, then this is a recursive call to malloc
        return nullptr;
    }
    if (realMalloc == nullptr) { // If realMalloc is null, then we need to interpose malloc
        isDlsym = true;
        realMalloc = (void *(*)(size_t)) dlsym(RTLD_NEXT, "malloc");
        if (realMalloc == nullptr) { // Make sure dlsym worked
            write(STDERR_FILENO, err, strlen(err));
            exit(EXIT_FAILURE);
        }
        isDlsym = false;
    }
    data = getData(); // Fetch rest of data

    ptr = realMalloc(size);
    data->dataMtx.lock();
    data->addrs.add((char *) &ptr, sizeof(void *));
    data->numAllocs++;
    if (data->numAllocs == data->nextIndex) { // Print data every power of two
        data->nextIndex <<= 1;
        localNumAllocs = data->numAllocs;
        cardinality = data->addrs.estimate();
        data->dataMtx.unlock();
        maxEntropy = log(localNumAllocs) / log(2.0);
        entropy = log(cardinality) / log(2.0);
        if (maxEntropy == 0) {
            percentage = 100;
        } else {
            percentage = entropy * 100.0 / maxEntropy;
        }
        data->writeMtx.lock();
        tprintf::tprintf(
            "libentroprise: Number of Allocations: @\n"
            "libentroprise: Number of Unique Addresses: @\n"
            "libentroprise: Calculated Entropy: @\n"
            "libentroprise: Maximum Entropy: @\n"
            "libentroprise: Percentage of Maximum: @%\n\n",
            localNumAllocs, cardinality, entropy, maxEntropy, percentage
        );
        data->writeMtx.unlock();
    } else {
        data->dataMtx.unlock();
    }
    return ptr;
}
