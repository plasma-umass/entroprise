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

    * HEAPLAYERS MODIFICATIONS:

    * Change FD to 3 in Heap-Layers/utility/tprintf.h

    * FREEGUARD/GUARDER MODIFICATIONS

    * In libfreeguard.cpp/libguarder.cpp, comment out the xxcalloc alias
    * Is this correct? Only uses glibc calloc...
    * Would have to intercept calloc myself
*/

class Data {
    public:
        Data() {
            char *err1 = (char *) "libentroprise: ERROR: cannot dlsym malloc\n";
            char *err2 = (char *) "libenroprise: ERROR: cannot create out file\n";
            realMalloc = (void *(*)(size_t)) dlsym(RTLD_NEXT, "malloc");
            if (realMalloc == nullptr) {
                write(STDERR_FILENO, err1, strlen(err1));
                exit(EXIT_FAILURE);
            }
            numAllocs = 0;
            nextIndex = 1;
        }

        void *(*realMalloc)(size_t);
        std::mutex mtx;
        hll::HyperLogLog addrs = hll::HyperLogLog(16);
        int numAllocs, nextIndex;
};

inline static Data *getData() {
    static char buf[sizeof(Data)];
    static Data *data = new (buf) Data;
    return data;
}

void *malloc(size_t size) {
    static Data *data;
    data = getData();
    int localNumAllocs;
    double cardinality, entropy, maxEntropy, percentage;
    void *ptr;

    ptr = data->realMalloc(size);
    data->mtx.lock();
    data->addrs.add((char *) &ptr, sizeof(void *));
    data->numAllocs++;
    if (data->numAllocs == data->nextIndex) { // Print data every power of two
        data->nextIndex <<= 1;
        localNumAllocs = data->numAllocs;
        cardinality = data->addrs.estimate();
        data->mtx.unlock();
        maxEntropy = log(localNumAllocs) / log(2.0);
        entropy = log(cardinality) / log(2.0);
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
            localNumAllocs, cardinality, entropy, maxEntropy, percentage
        );
    } else {
        data->mtx.unlock();
    }
    return ptr;
}
