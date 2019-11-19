// Executes runs test and KS with malloc

#include <iostream>
#include "runs.h"
#include "ks.h"

using namespace std;

void getAddrs(long unsigned int[], const int, const int);

int main(int argc, char *argv[]) {
 	const int NALLOCS = 1000, NTESTS = 100, SZ = 16;
 	long unsigned int addrs[NALLOCS];
 	double p[NTESTS], d, dAlpha = 0.565;
 
 	for (int i = 0; i < NALLOCS; ++i) {
 		addrs[i] = (long unsigned int) malloc(SZ);
 	}
	for (int i = 0; i < NALLOCS; ++i) {
		free((void *) addrs[i]);
	}

 	for (int i = 0; i < NTESTS; ++i) {
		getAddrs(addrs, NALLOCS, SZ); // Need to reset?
 		p[i] = runs(addrs, NALLOCS);
 	}
 	d = ks(p, NTESTS);
 
 	if (dAlpha > d) {
 		cout << "POSSIBLY RANDOM" << endl;
 	} else {
 		cout << "NOT RANDOM" << endl;
 	}
 
	return 0;
}

void getAddrs(long unsigned int addrs[], const int NALLOCS, const int SZ) {
	for (int i = 0; i < NALLOCS; ++i) {
 		addrs[i] = (long unsigned int) malloc(SZ);
		free((void *) addrs[i]);
	}
}
