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

bool parse_args(int argc, char *argv[], std::string *alloc, int *exec_i) {
    if (argc < 2) {
        return false;
    } else if (strncmp(argv[1], "-a", std::min(strlen(argv[1]), strlen("-a"))) == 0) {
        if (argc < 4) {
            return false;
        }
        alloc->assign(argv[2]);
        *(exec_i) = 3;
        return (access(alloc->c_str(), F_OK) != -1 && access(argv[*exec_i], F_OK) != -1);
    } else {
        *exec_i = 1;
        return (access(argv[*exec_i], F_OK) != -1);
    }
}

int main(int argc, char *argv[]) {
    std::string alloc;
    int exec_i;
    if (!parse_args(argc, argv, &alloc, &exec_i)) {
        std::cerr << "usage -a <alloc> <exec>" << std::endl;
        return EXIT_FAILURE;
    }

    double avg_num_allocs = 1, avg_card = 1, avg_entropy = 1, avg_max = 1, avg_normal = 1, min_normal, max_normal, std_dev_num_allocs = 0, std_dev_normal = 0;
    int total_num_allocs = 0, num_random = 0, min_num_allocs, max_num_allocs;
    std::vector<ParsedThreadData> *tdata;
    char fname[100];
    ParsedThreadData *cur;

    for (int i = 0; true; i++) {
        snprintf(fname, 100, THREAD_DIR "/" THREAD_FILE_PREFIX "%d" THREAD_FILE_POSTFIX, i);
        if (access(fname, F_OK) == -1) {
            break;
        }
        unlink(fname);
    }

    if (alloc.size() == 0) {
        create_proc(argv + exec_i, nullptr);
    } else {
        create_proc(argv + exec_i, (char *) alloc.c_str());
    }
    tdata = get_child_data();

    using namespace std;

    cout << endl << endl << "ENTROPRISE RESULTS" << endl;
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
    cout << "\t" << num_random << " / " << tdata->size() << " Random Sequences" << endl << endl;

    return EXIT_SUCCESS;
}
