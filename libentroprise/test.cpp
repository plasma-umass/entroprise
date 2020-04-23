#include <iostream>
#include <cstdlib>
#include <thread>
#include <cstring>

void func(const int NUM_ITERS, const int OBJ_SIZE) {
    void *addrs[NUM_ITERS];
    for (int i = 0; i < NUM_ITERS; i++) {
        addrs[i] = malloc(OBJ_SIZE);
        memset(addrs[i], 'a', OBJ_SIZE);
        free(addrs[i]);
     }
 }

 int main(int argc, char *argv[]) {
    if (argc != 4) {
        std::cerr << "usage: <nthreads> <niters> <objsize>" << std::endl;
        return -1;
    }
    const int NUM_THREADS = std::stoi(argv[1]), NUM_ITERS = std::stoi(argv[2]), OBJ_SIZE = std::stoi(argv[3]);
    std::thread *threads = new std::thread[NUM_THREADS];
    std::cout << "Starting test..." << std::endl;
    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i] = std::thread(func, NUM_ITERS, OBJ_SIZE);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i].join();
    }
    std::cout << "Ending test..." << std::endl;
    return 0;
 }
