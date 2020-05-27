// Executes runs test and KS with malloc

#include <iostream>
#include <cstdlib>
#include <cstdint>
#include "runs.h"
#include "ks.h"

using namespace std;

void getAddrs(long unsigned int[], const int, const int);
void printDist(double p[], const int NTESTS);

int main() {
 	// const int NALLOCS = 10000, NTESTS = 100, SZ = 16;
 	// long unsigned int *addrs = new long unsigned int [NALLOCS * 100];
 	const int NALLOCS = 10000, NTESTS = 1, SZ = 16;
 	long unsigned int *addrs = new long unsigned int [NALLOCS];
 	double p[NTESTS], d, dAlpha = 0.565;
 
 	for (int i = 0; i < NALLOCS; ++i) {
 		addrs[i] = (long unsigned int) malloc(SZ);
 	}
	for (int i = 0; i < NALLOCS; ++i) {
		free((void *) addrs[i]);
	}

	cout << "RUNS TEST (" << NTESTS << " TRIALS)" << endl;
 	for (int i = 0; i < NTESTS; ++i) {
		// getAddrs(addrs, NALLOCS * 100, SZ);
 		// p[i] = runs(addrs, NALLOCS * 100);
		getAddrs(addrs, NALLOCS, SZ);
 		p[i] = runs(addrs, NALLOCS);
		cout << p[i] << endl;
 	}
	return 0;
	cout << endl << "P-VALUE DISTRIBUTION" << endl;
	printDist(p, NTESTS);
	cout << endl << "KOLMOGOROV-SMIRNOV TEST" << endl;
 	d = ks(p, NTESTS);
	cout << "MAX DISTANCE FROM UNIFORM DISTRIBUTION CDF: " << d << endl;
 
 	if (dAlpha > d) {
 		cout << endl << "ALLOCATOR IS POSSIBLY RANDOM" << endl;
 	} else {
 		cout << endl << "ALLOCATOR IS NOT RANDOM" << endl;
 	}
 
	return 0;
}

void getAddrs(long unsigned int addrs[], const int NALLOCS, const int SZ) {
	for (int i = 0; i < NALLOCS ; ++i) {
 		addrs[i] = (long unsigned int) malloc(SZ);
		free((void *) addrs[i]);
	}
}

void printDist(double p[], const int NTESTS) {
	int ct[5];
	for (int i = 0; i < 5; ++i) {
		ct[i] = 0;
	}
	for (int i = 0; i < NTESTS; ++i) {
		if (p[i] == 1) {
			++ct[4];
			continue;
		}
		intptr_t cur = (intptr_t) (p[i] * 100) / 20;
		++ct[cur];
	}
	printf(
		"0.0 - 0.2: %d\n"
		"0.2 - 0.4: %d\n"
		"0.4 - 0.6: %d\n"
		"0.6 - 0.8: %d\n"
		"0.8 - 1.0: %d\n",
		ct[0], ct[1], ct[2], ct[3], ct[4]
	);
}
