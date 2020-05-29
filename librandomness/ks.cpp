#include <iostream>
#include <cmath>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "runs.hh"
#include "ks.hh"

extern char **environ;

void fatal() {
    perror("ERROR");
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "usage <ld_preload> <exec>\n");
        return EXIT_FAILURE;
    }

    char **new_environ;
    long unsigned int *addrs;
    struct stat stat_buf;
    int fd, i, num_addrs, seq_len;
    const int NUM_RUNS_TESTS = 100;
    const double D_ALPHA = 0.565;
    double p[NUM_RUNS_TESTS], d;

    if (fork() == 0) {
        i = 0;
        for (i = 0; environ[i] != nullptr; i++); // Find number of environment variables so we know how much to malloc
        new_environ = (char **) malloc((i + 2) * sizeof(char *)); // +1 for LD_PRELOAD and +1 for nullptr
        new_environ[0] = argv[1]; // Copy LD_PRELOAD value to environment variables of new process
        for (i = 0; environ[i] != nullptr; i++) {
            new_environ[i + 1] = environ[i];
        }
        new_environ[i + 1] = nullptr;
        execve(argv[2], argv + 2, new_environ);
        fatal(); // execve should never reach this point if it succeeded
    }
    wait(nullptr);

    fd = open("addrs.bin", O_RDWR); // Open the same file that librandomness.so saved addresses to
    if (fd == -1) {
        fatal();
    }
    if (fstat(fd, &stat_buf) == -1) {
        fatal();
    }
    addrs = (long unsigned int *) mmap(nullptr, stat_buf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addrs == MAP_FAILED) {
        fatal();
    }
    if (madvise(addrs, stat_buf.st_size, MADV_SEQUENTIAL) == -1) {
        fatal();
    }
    num_addrs = *((int *) addrs); // The number of addresses is stored at the beginning of the mapping
    addrs = (long unsigned int *) ((int *) addrs + 1); // Following the number of addresses are the addresses themselves

    seq_len = ceil(num_addrs / NUM_RUNS_TESTS); // The number of allocations for each runs test
    for (i = 0; i < NUM_RUNS_TESTS; i++) {
        p[i] = runs(addrs + i * seq_len, seq_len); // Perform a runs test on each sequence and save the p-value
    }

    d = ks(p, NUM_RUNS_TESTS); // Perform a KS test on the p-values
    if (D_ALPHA > d) {
 		std::cout << "ALLOCATOR IS POSSIBLY RANDOM" << std::endl;
    } else {
 		std::cout << "ALLOCATOR IS NOT RANDOM" << std::endl;
    }

    munmap(addrs, stat_buf.st_size);
    close(fd);
    return EXIT_SUCCESS;
}
