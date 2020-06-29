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
    if (argc < 4) {
        fprintf(stderr, "usage <num_allocs> <ld_preload> <exec>\n");
        return EXIT_FAILURE;
    }

    const int MAX_ADDRS = std::stoi(argv[1]), DEFAULT_FILE_SIZE = sizeof(int) + sizeof(hll::HyperLogLog) + sizeof(void *) * MAX_ADDRS;
    void *ptr;
    int fd, num_allocs, seq_len;
    hll::HyperLogLog *h;
    double card, entropy, max, ratio;
    long unsigned int *addrs;
    const int NUM_RUNS_TESTS = 100;
    const double D_ALPHA = 0.565;
    double p[NUM_RUNS_TESTS], d;
    int runs_data[NUM_RUNS_TESTS][3];

    create_process(argv + 3, environ, argv[2]);
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
        p[i] = runs(addrs + i * seq_len, seq_len, runs_data[i]); // Perform a runs test on each sequence and save the p-value
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
        cout << "\t\tnum_plus = " << runs_data[i][0] << endl;
        cout << "\t\tnum_neg = " << runs_data[i][1] << endl;
        cout << "\t\tnum_runs = " << runs_data[i][2] << endl;

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
