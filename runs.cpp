// Runs Test: https://www.itl.nist.gov/div898/handbook/eda/section3/eda35d.htm
// Kolmogorov-Smirnov: https://www.itl.nist.gov/div898/handbook/eda/section3/eda35g.htm
// Kolmogorov-Smirnov Implementation: https://stackoverflow.com/questions/37625280/kolmogorov-smrinov-test-in-c

#include <iostream>
#include <algorithm>
#include <random>
#include "runs.h"

using namespace std;

int main(int argc, char *argv[]) {
	const unsigned int NRANDS = 1000, NTESTS = 100;
	double p[NTESTS], d, dAlpha = 0.565; // Assume alpha = 0.05
	random_device rd;

	for (int i = 0; i < NTESTS; ++i) { // Execute runs test NTESTS times and store p-values
		p[i] = runsTest(rd(), NRANDS);
		cout << "p-value #" << i + 1 << " = " << p[i] << endl;
	}
	d = ksTest(p, NTESTS); // Run Kolmogorov-Smirnov test to compare with uniform distribution

	cout << endl << "D = " << d << endl;
	if (dAlpha > d) {
		cout << "POSSIBLY RANDOM" << endl;
	} else {
		cout << "NOT RANDOM" << endl;
	}

	return 0;
}

double runsTest(const unsigned int SEED, const unsigned int N) {
	mt19937 mt(SEED);
	uniform_int_distribution<unsigned int> dist;
	unsigned int a[N], tmp[N];

	for (int i = 0; i < N; ++i) { // Fill with random values
		a[i] = dist(mt);
		tmp[i] = a[i];
	}

	sort(tmp, tmp + N); // Sort to find median
	unsigned int median = (tmp[N / 2 - 1] + tmp[N / 2]) / 2; // Assume N is even
	int sz = 0, nPlus = 0, nNeg = 0;

	for (int i = 0; i < N; ++i) { // Discard elements equal to median and get number of vales greater or less than median
		if (a[i] != median) {
			if (a[i] > median) ++nPlus;
			else ++nNeg;
			tmp[sz++] = a[i];
		}
	}

	int nRuns = (sz != 0) ? 1 : 0; // If there is at least one element, there will be at least 1 run. Otherwise, there are no runs.
	for (int i = 0; i < sz - 1; ++i) { // Find number of runs
		bool b1 = (tmp[i] > median), b2 = (tmp[i + 1] > median);
		if ((b1 && !b2) || (!b1 && b2)) { // If moving from + to - or - to +
			++nRuns;	
		}
	}

	if (nPlus == 0 || nNeg == 0) { // Will cause division by zero
		return 0;
	}

	long double expected = (long double) 2 * nPlus * nNeg / (nPlus + nNeg) + 1; // Cast for overflow (these numbers get very big)
	long double variance = (long double) 2 * nPlus * nNeg * (2 * nPlus * nNeg - nPlus - nNeg); // Cast again
	variance /= (long double) (nPlus + nNeg) * (nPlus + nNeg) * (nPlus + nNeg - 1);
	double Z = (nRuns - expected) / sqrt(variance);

	if (abs(Z) >= 4.1) { // To prevent overread in table
		return 0;
	}
	 
	return 1 - 2 * table[(int) (abs(Z) * 100)]; // Two-tailed hypthesis, Look up value in standard normal table
}

double ksTest(double num[], int length) {
	sort(num, num + length); // Must sort data
	
	double d[length], n = (double) length, maxPlus, maxNeg; // d stores differences between ECDF and uniform distribution CDF

	for (int i = 0; i < length; ++i) { // Compute maximum distance D+ when uniform distribution is above EDF
		d[i] = ((i + 1) / n) - num[i]; // Is this correct? ECDF - uniform distribution CDF?
	}
	maxPlus = *max_element(d, d + length);

	for (int i = 0; i < length; ++i) { // Compute maximum distance D- when uniform distribution is below EDF
		d[i] = (num[i] - (i) / n); // Is this correct? uniform distribution CDF - ECDF?
	}
	maxNeg = *max_element(d, d + length);
	
	return max(maxPlus, maxNeg); // Return D
}

void fatal(string str) { // Terminate program
	cerr << str << endl;
	exit(-1);
}

void printRuns(unsigned int *a, const int N, unsigned int median) { // For debugging
	for (int i = 0; i < N; ++i) {
		cout << a[i] << " ";
		if (a[i] > median) cout << "+" << endl;
		else if(a[i] < median) cout << "-" << endl;
		else cout << "equal" << endl;
	}
}
