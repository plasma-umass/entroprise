#include <iostream>
#include <cstdlib>
#include <thread>
#include <cstring>

inline void *ec_malloc(size_t size) {
    void *ptr = malloc(size);
    if (ptr == nullptr) {
        std::cerr << "test ERROR: malloc failed" << std::endl;
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void no_reuse(const int NUM_ITERS, const int OBJ_SIZE) {
    void *addrs[NUM_ITERS];
    for (int i = 0; i < NUM_ITERS; i++) {
        addrs[i] = ec_malloc(OBJ_SIZE);
    }
    for (int i = 0; i < NUM_ITERS; i++) {
        free(addrs[i]);
    }
}

void all_reuse(const int NUM_ITERS, const int OBJ_SIZE) {
    void *ptr;
    for (int i = 0; i < NUM_ITERS; i++) {
        ptr = ec_malloc(OBJ_SIZE);
        free(ptr);
     }
}

int main(int argc, char *argv[]) {
    if (argc != 5 || (strcmp(argv[1], (char *) "no_reuse") != 0 && strcmp(argv[1], (char *) "all_reuse") != 0)) {
        std::cerr << "usage: <no_reuse/all_reuse> <num_threads> <num_iters> <obj_size>" << std::endl;
        return -1;
    }
    const int NUM_THREADS = std::stoi(argv[2]), NUM_ITERS = std::stoi(argv[3]), OBJ_SIZE = std::stoi(argv[4]);
    std::thread *threads = new std::thread[NUM_THREADS];
    void (*func)(const int, const int);
    if (strcmp(argv[1], "no_reuse") == 0) {
        func = no_reuse;
    } else {
        func = all_reuse;
    }
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
