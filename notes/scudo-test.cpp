#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <unordered_map>
#include <heaplayers>
#include <vector>
#include <algorithm>
#define MY_LIST_MAX 1000

/*
    * To run without quarantine:
    * SCUDO_OPTIONS="QuarantineSizeKb=0:ThreadLocalQuarantineSizeKb=0" LD_PRELOAD=/home/msteranka/scratch/shared/libscudo.so ./scudo-test

    * To quarantine chunks up to 20 bytes:
    * SCUDO_OPTIONS="QuarantineChunksUpToSize=20" LD_PRELOAD=/home/msteranka/scratch/shared/libscudo.so ./scudo-test

    * Scudo exhibits randomness with object placement
    * Completely deterministic object reuse if quarantine is disabled
    
    * If quarantine is enabled, object reuse is still deterministic? Maybe?
    * When the quarantine cache is full, the last 8 objects put into quarantine are the first 8 to come out when allocating
    * e.g.
    * BEFORE QUARANTINE IS FULL
    * 0x0
    * 0x1
    * 0x2
    * 0x3
    * AFTER QUARANTINE IS FULL
    * 0x3
    * 0x2
    * 0x1
    * 0x0
    * After those first 8 objects, the first 4 objects put into quarantine are the next 4 to come out
    * After that, addresses are random again?

    * < 52 Allocations
    * No reuse
    * Quarantine becomes full

    * <= 52 Allocations < 104 Allocations
    * Last 8 objects put into quarantine are the first 8 to come out when allocating
    * First 4 objects put into quarantine are the next 4 to come out
    * Next 4 addresses are new addresses
    * Next 35 addresses are the 35 addresses that were put into quarantine right before the first 8 objects that came out then come out when allocating
    * 52 - 59
    * 60 - 63
    * 68 - 103

    * <= 104 Allocations < 
    * 104 - 111
    * 112 - 115
    * 116 - 119

    * Addresses that can be reused is bounded by the size of the quarantine cache
*/

template <typename T>
class MyAllocator : public STLAllocator<T, FreelistHeap<BumpAlloc<4096, MmapHeap>>> {};

class MyList {
    public:
        MyList() {
            size = 0;
        }

        void append(int n) {
            if (size >= MY_LIST_MAX) {
                abort();
            }
            a[size++] = n;
        }

        int at(int i) {
            if (i >= MY_LIST_MAX) {
                abort();
            }
            return a[i];
        }

        void print() {
            std::cout << "[";
            for (int i = 0; i < size - 1; i++) {
                std::cout << a[i] << ", ";
            }
            std::cout << a[size - 1] << "]";
        }

    private:
        int a[MY_LIST_MAX], size;
};

bool cmp(std::pair<void *, MyList> a, std::pair<void *, MyList> b) {
    return (a.second.at(0) < b.second.at(0));
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "usage <num_allocs>" << std::endl;
        return -1;
    }

    // void *prev = nullptr, *cur;
    const int N = std::stoi(argv[1]);
    void *cur;
	std::unordered_map<void *, MyList, hash<void *>, equal_to<void *>, MyAllocator<pair<void * const, MyList>>> m;
	// std::unordered_map<void *, std::vector<int, MyAllocator<int>>, hash<void *>, equal_to<void *>, MyAllocator<pair<void * const, std::vector<int, MyAllocator<int>>>>> m;
	// std::unordered_map<void *, std::vector<int>, hash<void *>, equal_to<void *>, MyAllocator<pair<void * const, std::vector<int>>>> m;
    std::vector<pair<void *, MyList>> sorted_v;

    for (int i = 0; i < N; i++) {
        cur = malloc(20);
        free(cur);
        tprintf::tprintf("@: cur = @\n", i, cur);
        // printf("%d: cur = %p\n", i, cur);

        // if (m.find(cur) != m.end()) {
        //     printf("SAME ADDRESS, i = %d\n", i);
        //     ct++;
        //     if (ct == 10) {
        //         return 0;
        //     }
        // }
        m[cur].append(i);
        // m[cur].push_back(i);
        // if (!prev) {
        //     printf("prev = nullptr, cur = %p\n", cur);
        // } else {
        //     printf("prev = %p, cur = %p, diff = %ld\n", prev, cur, (long) cur - (long) prev);
        // }
        // prev = cur;
    }
    for (auto it = m.begin(); it != m.end(); it++) {
        sorted_v.emplace_back(it->first, it->second);
    }
    std::sort(sorted_v.begin(), sorted_v.end(), cmp);
    for (auto it = sorted_v.begin(); it != sorted_v.end(); it++) {
        printf("%p: ", it->first);
        it->second.print();
        printf("\n");
    }
    printf("%d\n", sorted_v.size());

    // for (auto it1 = m.begin(); it1 != m.end(); it1++) {
    //     printf("%p: ", it1->first);
    //     it1->second.print();
    //     // for (auto it2 = it1->second.begin(); it2 != it1->second.end(); it2++) {
    //     //     printf("%d ", *it2);
    //     // }
    //     printf("\n");
    // }

    return 0;
}
