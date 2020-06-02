#include <iostream>
#include <cstdlib>
#include <cmath>
#include "proc.hh"

extern char **environ;

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "usage <ld_preload> <exec>\n");
        return EXIT_FAILURE;
    }

    void *ptr;
    int num_allocs;
    double card, entropy, max, ratio;

    create_process(argv + 2, environ, argv[1]);
    ptr = get_proc_data();
    num_allocs = *((int *) ptr);
    card = *((double *) ((int *) ptr + 1));
    entropy = log(card) / log(2.0);
    max = log(num_allocs) / log(2.0);
    ratio = entropy / max;

    std::cout << "Number of Allocations: " << num_allocs << std::endl;
    std::cout << "Number of Unique Addresses: " << card << std::endl;
    std::cout << "Calculated Entropy: " << entropy << std::endl;
    std::cout << "Maximum Entropy: " << max << std::endl;
    std::cout << "Normalized Entropy: " << ratio << std::endl;

    return EXIT_SUCCESS;
}
