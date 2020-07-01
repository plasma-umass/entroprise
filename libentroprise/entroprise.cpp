#include <iostream>
#include <cstdlib>
#include <vector>
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

    // const int MAX_ADDRS = std::stoi(argv[1]), DEFAULT_FILE_SIZE = sizeof(int) + sizeof(hll::HyperLogLog) + sizeof(void *) * MAX_ADDRS;
    // void *ptr;
    // int fd, num_allocs, seq_len;
    // hll::HyperLogLog *h;
    // double card, entropy, max, ratio;
    // long unsigned int *addrs;
    // const int NUM_RUNS_TESTS = 100;
    // const double D_ALPHA = 0.565;
    // double p[NUM_RUNS_TESTS], d;
    // int runs_data[NUM_RUNS_TESTS][3];
    double avg_num_allocs = 1, avg_card = 1, avg_entropy = 1, avg_max = 1, avg_normal = 1, min_normal, max_normal, std_dev_num_allocs = 0, std_dev_normal = 0;
    int total_num_allocs = 0, num_random = 0, min_num_allocs, max_num_allocs;
    std::vector<ParsedThreadData> *tdata;
    char fname[100];
    ParsedThreadData *cur;

    for (int i = 0; true; i++) {
        snprintf(fname, 100, "%d.threads.bin", i);
        if (access(fname, F_OK) == -1) {
            break;
        }
        unlink(fname);
    }

    create_process(argv + 3, environ, argv[2]);
    tdata = get_child_data();

    // num_allocs = *((int *) ptr);
    // h = new((int *) ptr + 1) hll::HyperLogLog((char *) nullptr);
    // addrs = (long unsigned int *) (h + 1);

    // card = h->estimate();
    // entropy = log(card) / log(2.0);
    // max = log(num_allocs) / log(2.0);
    // ratio = entropy / max;

    // seq_len = ceil(num_allocs / NUM_RUNS_TESTS); // The number of allocations for each runs test
    // for (int i = 0; i < NUM_RUNS_TESTS; i++) {
    //     p[i] = runs(addrs + i * seq_len, seq_len, runs_data[i]); // Perform a runs test on each sequence and save the p-value
    // }
    // d = ks(p, NUM_RUNS_TESTS); // Perform a KS test on the p-values

    using namespace std;
    // cout << endl << "ENTROPRISE RESULTS" << endl;
    // cout << "------------------------------------------------------------" << endl;
    // cout << "NUMBER OF ALLOCATIONS: " << num_allocs << endl << endl;
    // cout << "ENTROPY DATA:" << endl;
    // cout << "\tNumber of Unique Addresses: " << card << endl;
    // cout << "\tCalculated Entropy: " << entropy << endl;
    // cout << "\tMaximum Entropy: " << max << endl;
    // cout << "\tNormalized Entropy: " << ratio << endl << endl;
    // cout << "RUNS TESTS RESULTS:" << endl;
    // for (int i = 0; i < NUM_RUNS_TESTS; i++) {
    //     cout << "\tRun Test #";
    //     if (i + 1 < 10) {
    //         cout << "0";
    //     }
    //     cout << i + 1 << ": p-value = " << p[i] << endl;
    //     cout << "\t\tnum_plus = " << runs_data[i][0] << endl;
    //     cout << "\t\tnum_neg = " << runs_data[i][1] << endl;
    //     cout << "\t\tnum_runs = " << runs_data[i][2] << endl;

    // }
    // cout << endl << "KOLMOGOROV-SMIRNOV TEST RESULTS:" << endl;
    // cout << "\tD = " << d << endl;
    // if (D_ALPHA > d) {
 	// 	cout << "\tALLOCATOR IS POSSIBLY RANDOM" << endl;
    // } else {
 	// 	cout << "\tALLOCATOR IS NOT RANDOM" << endl;
    // }
    // cout << endl << "SUMMARY:" << endl;
    // cout << "\tNormalized Entropy: " << ratio << endl;
    // if (D_ALPHA > d) {
 	// 	cout << "\tALLOCATOR IS POSSIBLY RANDOM" << endl;
    // } else {
 	// 	cout << "\tALLOCATOR IS NOT RANDOM" << endl;
    // }

    cout << endl << "ENTROPRISE RESULTS" << endl;
    cout << "------------------------------------------------------------" << endl;
    if (tdata->size() == 0) {
        cout << "No data available. malloc was never called.\n" << endl;
        return EXIT_SUCCESS;
    }
    min_num_allocs = max_num_allocs = tdata->at(0).num_allocs;
    min_normal = max_normal = tdata->at(0).ratio;
    for (int i = 0; i < tdata->size(); i++) {
        cur = &(tdata->at(i));
        total_num_allocs += cur->num_allocs;
        avg_num_allocs *= cur->num_allocs;
        avg_card *= cur->card;
        avg_entropy *= cur->entropy;
        avg_max *= cur->max;
        avg_normal *= cur->ratio;
        min_num_allocs = std::min(min_num_allocs, cur->num_allocs);
        max_num_allocs = std::max(max_num_allocs, cur->num_allocs);
        min_normal = std::min(min_normal, cur->ratio);
        max_normal = std::max(max_normal, cur->ratio);

        cout << "THREAD " << i << ":" << endl;
        cout << "\tNumber of Allocations: " << cur->num_allocs << endl;
        cout << "\tNumber of Unique Addresses: " << cur->card << endl;
        cout << "\tCalculated Entropy: " << cur->entropy << endl;
        cout << "\tMaximum Entropy: " << cur->max << endl;
        cout << "\tNormalized Entropy: " << cur->ratio << endl;
        if (cur->is_random) {
            cout << "\tSequence is POSSIBLY random" << endl << endl;
            num_random++;
        } else {
            cout << "\tSequence is NOT random" << endl << endl;
        }
    }

    avg_num_allocs = pow(avg_num_allocs, 1.0 / tdata->size());
    avg_card = pow(avg_card, 1.0 / tdata->size());
    avg_entropy = pow(avg_entropy, 1.0 / tdata->size());
    avg_max = pow(avg_max, 1.0 / tdata->size());
    avg_normal = pow(avg_normal, 1.0 / tdata->size());

    for (int i = 0; i < tdata->size(); i++) { // Iterate through again to calculate standard deviations
        cur = &(tdata->at(i));
        std_dev_num_allocs += pow(cur->num_allocs - avg_num_allocs, 2);
        std_dev_normal += pow(cur->ratio - avg_normal, 2);
    }

    std_dev_num_allocs = sqrt(std_dev_num_allocs / tdata->size());
    std_dev_normal = sqrt(std_dev_normal / tdata->size());

    cout << "SUMMARY:" << endl;
    cout << "\tTotal Number of Threads: " << tdata->size() << endl;
    cout << "\tTotal Number of Allocations: " << total_num_allocs << endl;
    cout << endl;
    cout << "\tAverage Number of Allocations: " << avg_num_allocs << endl;
    cout << "\tAverage Number of Unique Addresses: " << avg_card << endl;
    cout << "\tAverage Calculated Entropy: " << avg_entropy << endl;
    cout << "\tAverage Maximum Entropy: " << avg_max << endl;
    cout << "\tAverage Normalized Entropy: " << avg_normal << endl;
    cout << endl;
    cout << "\tMinimum Number of Allocations: " << min_num_allocs << endl;
    cout << "\tMaximum Number of Allocations: " << max_num_allocs << endl;
    cout << "\tMinimum Normalized Entropy: " << min_normal << endl;
    cout << "\tMaximum Normalized Entropy: " << max_normal << endl;
    cout << "\tNumber of Allocations Standard Deviation: " << std_dev_num_allocs << endl;
    cout << "\tNormalized Entropy Standard Deviation: " << std_dev_normal << endl;
    cout << endl;
    cout << "\t" << num_random << " / " << tdata->size() << " Random Sequences" << endl;

    return EXIT_SUCCESS;
}
