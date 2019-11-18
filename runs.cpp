// Source: https://www.itl.nist.gov/div898/handbook/eda/section3/eda35d.htm

#include <iostream>
#include <algorithm>
#include "runs.h"

using namespace std;

int main(int argc, char *argv[]) {
	if (argc != 2) {
		fatal("usage: <N>");
	}

	const unsigned int N = stoi(argv[1]);

	if (N % 2 != 0) { // N is assumed to be even when calculating median
		fatal("N must be even");
	} else if (N > 10000) { // Must restrict value of N to prevent overflow when calculating mean and variance
		fatal("N cannot be greater than 10000");
	}

	srand(time(NULL));
	unsigned int *a = new unsigned int[N], *tmp = new unsigned int[N];

	for (int i = 0; i < N; ++i) { // Fill with random values
		a[i] = rand();
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

	if (nPlus + nNeg == 0 || nPlus + nNeg == 1) { // Will cause division by zero
		cerr << "Cannot calculate Z-score and p-value" << endl;
		return -1;
	}

	long double expected = (long double) 2 * nPlus * nNeg / (nPlus + nNeg) + 1; // Cast for overflow
	long double variance = (long double) 2 * nPlus * nNeg * (2 * nPlus * nNeg - nPlus - nNeg); // Cast again for same reason
	variance /= (long double) (nPlus + nNeg) * (nPlus + nNeg) * (nPlus + nNeg - 1);
	double Z = (nRuns - expected) / sqrt(variance), alpha = 0.05, p;

	if (abs(Z) >= 4.1) { // To prevent overread in table HERE
		p = 0;
	} else {
		p = 1 - 2 * table[(int) (abs(Z) * 100)]; // Two-tailed hypthesis, Look up value in standard normal table
	}

	cout << "Z-score = " << Z << endl;
	cout << "p-value = " << p << endl << endl;
	if (p < alpha) {
		cout << "Data is not random at p < " << alpha << endl;
	} else {
		cout << "Result is not significant at p < " << alpha << endl;
	}

	return 0;
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
