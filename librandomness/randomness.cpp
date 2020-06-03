#include <iostream>
#include <cmath>
#include <cstdlib>
#include "proc.hh"
#include "runs.hh"
#include "ks.hh"

extern char **environ;

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "usage <ld_preload> <exec>\n");
        return EXIT_FAILURE;
    }

    long unsigned int *addrs;
    int num_addrs, seq_len;
    const int NUM_RUNS_TESTS = 100;
    const double D_ALPHA = 0.565;
    double p[NUM_RUNS_TESTS], d;

    create_process(argv + 2, environ, argv[1]);
    addrs = (long unsigned int *) get_proc_data();
    num_addrs = *((int *) addrs); // The number of addresses is stored at the beginning of the mapping
    addrs = (long unsigned int *) ((int *) addrs + 1); // Following the number of addresses are the addresses themselves
    seq_len = ceil(num_addrs / NUM_RUNS_TESTS); // The number of allocations for each runs test

    for (int i = 0; i < NUM_RUNS_TESTS; i++) {
        p[i] = runs(addrs + i * seq_len, seq_len); // Perform a runs test on each sequence and save the p-value
    }
    d = ks(p, NUM_RUNS_TESTS); // Perform a KS test on the p-values
    if (D_ALPHA > d) {
 		std::cout << "ALLOCATOR IS POSSIBLY RANDOM" << std::endl;
    } else {
 		std::cout << "ALLOCATOR IS NOT RANDOM" << std::endl;
    }

    return EXIT_SUCCESS;
}
