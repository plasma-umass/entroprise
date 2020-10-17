// Executes runs test and KS with Mersenne Twister PRNG

#include <iostream>
#include <random>
#include "runs.hh"
#include "ks.hh"

using namespace std;

int main(int argc, char *argv[]) {
	const int NRANDS = 10000, NTESTS = 100;
	long unsigned int a[NRANDS];
	double p[NTESTS], d, dAlpha = 0.565; // Assume alpha = 0.05
    int runs_data[3];
	random_device rd;
 	mt19937_64 mt;
 	uniform_int_distribution<long unsigned int> dist;

	for (int i = 0; i < NTESTS; ++i) { // Execute runs test NTESTS times and store p-values
		mt.seed(rd());
		for (int j = 0; j < NRANDS; ++j) {
			a[j] = dist(mt);
		}
		p[i] = runs(a, NRANDS, runs_data);
	}
	d = ks(p, NTESTS); // Run Kolmogorov-Smirnov test to compare with uniform distribution

	cout << endl << "D = " << d << endl;
	if (dAlpha > d) {
		cout << "POSSIBLY RANDOM" << endl;
	} else {
		cout << "NOT RANDOM" << endl;
	}

	return 0;
}
