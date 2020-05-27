#include <iostream>
#include <unordered_map>
#include <cmath>
#include <cstdlib>
#include <future>
#include <iomanip>
#include <list>
#include <map>
#include <mutex>
#include <heaplayers>
#include "hyperloglog.hpp"
#include "murmur3.h"

using namespace HL;
using namespace std;

template <typename T>
class MyAllocator : public STLAllocator<T, FreelistHeap<BumpAlloc<4096, MmapHeap>>> {};

void fatal() {
    perror("ERROR:");
    exit(-1);
}

void primeHeap(const int OBJECT_SIZE, const int MIN_ALLOC) {
	char **objs = new char *[MIN_ALLOC];

	for (int i = 0; i < MIN_ALLOC; i++) {
		objs[i] = (char *) malloc(OBJECT_SIZE);
        if (objs[i] == NULL) {
            fatal();
        }
	}
  	for (int i = 0; i < MIN_ALLOC; i++) {
		free(objs[i]);
	}

	delete [] objs;
}

float getExactEntropy(const int OBJECT_SIZE, const int MIN_ALLOC) {
	unordered_map<char *, int, hash<char *>, equal_to<char *>, MyAllocator<pair<char * const, int>>> counter;
    primeHeap(OBJECT_SIZE, MIN_ALLOC);

  	for (int i = 0; i < MIN_ALLOC; i++) {
	    char *p = (char *) malloc(OBJECT_SIZE);
        if (p == NULL) {
            fatal();
        }
	    free(p);
	    if (counter.find(p) == counter.end()) {
	      counter[p] = 0;
	    }
	    counter[p]++;
  	}

  	float entropy = 0.0;
  	for (auto it = counter.begin(); it != counter.end(); ++it) {
		char *address = (*it).first;
  	  	int count = (*it).second;
  	  	entropy += -log(count / (double) MIN_ALLOC) / log(2.0) * (count / (double) MIN_ALLOC);
  	}
	return entropy;
}

float getBoundedEntropy(const int OBJECT_SIZE, const int MIN_ALLOC) {
    hll::HyperLogLog hll(16); // Initialize HyperLogLog with 2^16 bit width
    primeHeap(OBJECT_SIZE, MIN_ALLOC);

    for (int i = 0; i < MIN_ALLOC; i++) {
	    void *ptr = (void *) malloc(OBJECT_SIZE);
        if (ptr == NULL) {
            fatal();
        }
	    free(ptr);
        hll.add((char *) &ptr, sizeof(void *));
    }

    return log(hll.estimate()) / log(2.0);
}

int main(int argc, char *argv[]) {
	if (argc != 5) {
		cerr << "usage: <OBJECT_SIZE> <MIN_ALLOC> <NTHREADS> <BOUND=y/n>" << endl;
		return -1;
	}

	const int OBJECT_SIZE = stoi(argv[1]), MIN_ALLOC = stoi(argv[2]), NTHREADS = stoi(argv[3]);
    bool approx = (argv[4][0] == 'y');
	future<float> *threads = new future<float>[NTHREADS];
	float entropy = 1, max = log(MIN_ALLOC / NTHREADS) / log(2.0);

    if (approx) { // Bound entropy
        for (int i = 0; i < NTHREADS; i++) {
            threads[i] = async(getBoundedEntropy, OBJECT_SIZE, MIN_ALLOC / NTHREADS);
        }
    } else { // Get exact entropy
        for (int i = 0; i < NTHREADS; i++) {
            threads[i] = async(getExactEntropy, OBJECT_SIZE, MIN_ALLOC / NTHREADS);
        }
    }

	for (int i = 0; i < NTHREADS; i++) { // Compute average
		entropy *= threads[i].get();
    }
	entropy = pow(entropy, 1.0 / NTHREADS);

	cout << fixed << setprecision(4) << entropy << "   " << max << "   " << entropy * 100.0 / max << "%" << endl;
  	return 0;
}
