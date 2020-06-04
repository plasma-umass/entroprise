#include <iostream>
#include <cstdlib>
#include <cmath>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "hyperloglog.hpp"
#include "proc.hh"
#include "runs.hh"
#include "ks.hh"

extern char **environ;

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "usage <ld_preload> <exec>\n");
        return EXIT_FAILURE;
    }

    char c;
    const int MAX_ADDRS = 10000, DEFAULT_FILE_SIZE = sizeof(int) + sizeof(hll::HyperLogLog) + sizeof(void *) * MAX_ADDRS;
    void *ptr;
    int fd, num_allocs, seq_len;
    hll::HyperLogLog *h;
    double card, entropy, max, ratio;
    long unsigned int *addrs;
    const int NUM_RUNS_TESTS = 100;
    const double D_ALPHA = 0.565;
    double p[NUM_RUNS_TESTS], d;

    c = '\0';
    fd = open(FILE_NAME, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH); // Open file that will store data
    if (fd == -1) {
        fatal();
    }
    if (lseek(fd, DEFAULT_FILE_SIZE - 1, SEEK_SET) == -1) {
        fatal();
    }
    if (write(fd, &c, 1) == -1)  { // lseek + write to increase the size of the file so that it can be used to sufficiently store all data
        fatal();
    }
    close(fd);

    create_process(argv + 2, environ, argv[1]);
    ptr = get_proc_data();

    num_allocs = *((int *) ptr);
    h = new((int *) ptr + 1) hll::HyperLogLog((char *) nullptr);
    addrs = (long unsigned int *) (h + 1);

    card = h->estimate();
    entropy = log(card) / log(2.0);
    max = log(num_allocs) / log(2.0);
    ratio = entropy / max;

    seq_len = ceil(num_allocs / NUM_RUNS_TESTS); // The number of allocations for each runs test
    for (int i = 0; i < NUM_RUNS_TESTS; i++) {
        p[i] = runs(addrs + i * seq_len, seq_len); // Perform a runs test on each sequence and save the p-value
    }
    d = ks(p, NUM_RUNS_TESTS); // Perform a KS test on the p-values

    using namespace std;
    cout << endl << "ENTROPRISE RESULTS" << endl;
    cout << "------------------------------------------------------------" << endl;
    cout << "NUMBER OF ALLOCATIONS: " << num_allocs << endl << endl;
    cout << "ENTROPY DATA:" << endl;
    cout << "\tNumber of Unique Addresses: " << card << endl;
    cout << "\tCalculated Entropy: " << entropy << endl;
    cout << "\tMaximum Entropy: " << max << endl;
    cout << "\tNormalized Entropy: " << ratio << endl << endl;
    cout << "RUNS TESTS RESULTS:" << endl;
    for (int i = 0; i < NUM_RUNS_TESTS; i++) {
        cout << "\tRun Test #";
        if (i + 1 < 10) {
            cout << "0";
        }
        cout << i + 1 << ": p-value = " << p[i] << endl;
    }
    cout << endl << "KOLMOGOROV-SMIRNOV TEST RESULTS:" << endl;
    cout << "\tD = " << d << endl;
    if (D_ALPHA > d) {
 		cout << "\tALLOCATOR IS POSSIBLY RANDOM" << endl;
    } else {
 		cout << "\tALLOCATOR IS NOT RANDOM" << endl;
    }
    cout << endl << "SUMMARY:" << endl;
    cout << "\tNormalized Entropy: " << ratio << endl;
    if (D_ALPHA > d) {
 		cout << "\tALLOCATOR IS POSSIBLY RANDOM" << endl;
    } else {
 		cout << "\tALLOCATOR IS NOT RANDOM" << endl;
    }

    return EXIT_SUCCESS;
}
