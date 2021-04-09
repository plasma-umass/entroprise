#include <iostream>
#include <unordered_map>
#include <future>
#include <iomanip>
#include <cstdlib>
#include <cassert>
#include "hyperloglog.hpp"
#include "murmur3.h"
#include "heaplayers.h"

template <typename T>
class MyAllocator : public HL::STLAllocator<T, HL::FreelistHeap<HL::BumpAlloc<4096, HL::MmapHeap>>> { };

void prime_heap(const int OBJECT_SIZE, const int NUM_ALLOCS) {
    // Dynamically allocated because stack allocation can segfault
    //
    char **objs = new char *[NUM_ALLOCS];

    for (int i = 0; i < NUM_ALLOCS; i++) {
        objs[i] = (char *) malloc(OBJECT_SIZE);
        assert(objs[i]);
    }
    for (int i = 0; i < NUM_ALLOCS; i++) {
        free(objs[i]);
    }
    
    delete[] objs;
}

double get_exact_entropy(const int OBJECT_SIZE, const int NUM_ALLOCS) {
    std::unordered_map<char *, int, std::hash<char *>, std::equal_to<char *>, 
                       MyAllocator<std::pair<char * const, int>>> counter;
    double entropy;
    prime_heap(OBJECT_SIZE, NUM_ALLOCS);

    for (int i = 0; i < NUM_ALLOCS; i++) {
        char *ptr = (char *) malloc(OBJECT_SIZE);
        assert(ptr);
        if (counter.find(ptr) == counter.end()) {
            counter[ptr] = 0;
        }
        counter[ptr]++;
        free(ptr);
    }

    entropy = 0;
    for (auto it = counter.begin(); it != counter.end(); it++) {
        char *address = (*it).first;
        int count = (*it).second;
        entropy += -log(count / (double) NUM_ALLOCS) / log(2.0) * (count / (double) NUM_ALLOCS);
    }
    return entropy;
}

double get_approx_entropy(const int OBJECT_SIZE, const int NUM_ALLOCS, const uint8_t BIT_WIDTH) {
    // Larger BIT_WIDTH = more accuracy, but more space and longer runtime
    //
    hll::HyperLogLog hll(BIT_WIDTH);
    prime_heap(OBJECT_SIZE, NUM_ALLOCS);
    
    for (int i = 0; i < NUM_ALLOCS; i++) {
        char *ptr = (char *) malloc(OBJECT_SIZE);
        assert(ptr);
        hll.add((const char *) &ptr, sizeof(ptr));
        free(ptr);
    }
    
    return std::log(hll.estimate()) / log(2.0);
}

int main(int argc, char *argv[]) {
    if (argc != 5 && argc != 6) {
        std::cerr << "usage: <OBJECT_SIZE> <NUM_ALLOCS> <NUM_THREADS> <APPROXIMATE=y/n> <BIT_WIDTH>" << std::endl;
        return EXIT_FAILURE;
    }
    
    const unsigned int OBJECT_SIZE = std::stoi(argv[1]), 
                       NUM_ALLOCS = std::stoi(argv[2]), 
                       NUM_THREADS = std::stoi(argv[3]),
                       DEFAULT_BIT_WIDTH = 4;
    const bool IS_APPROX = (argv[4][0] == 'y');
    uint8_t bit_width;
    if (IS_APPROX) {
        if (argc == 6) {
            bit_width = std::stoi(argv[5]);
        } else {
            bit_width = DEFAULT_BIT_WIDTH;
        }
    }
    const double MAX_ENTROPY = std::log(NUM_ALLOCS) / std::log(2.0);
    std::future<double> threads[NUM_THREADS];
    double entropy;
    
    if (IS_APPROX) { // Get approximate entropy using HyperLogLog
        for (int i = 0; i < NUM_THREADS; i++) {
            threads[i] = std::async(get_approx_entropy, OBJECT_SIZE, NUM_ALLOCS, bit_width);
        }
    } else { // Get exact entropy using std::unordered_map
        for (int i = 0; i < NUM_THREADS; i++) {
            threads[i] = std::async(get_exact_entropy, OBJECT_SIZE, NUM_ALLOCS);
        }
    }
    
    entropy = 1;
    for (int i = 0; i < NUM_THREADS; i++) { // Compute geometric average
        entropy *= threads[i].get();
    }
    entropy = pow(entropy, 1.0 / NUM_THREADS);
    
    std::cout << std::fixed << std::setprecision(4) << entropy << "   " << MAX_ENTROPY << "   " << 
                 entropy / MAX_ENTROPY << std::endl;
                 
    return EXIT_SUCCESS;
}
