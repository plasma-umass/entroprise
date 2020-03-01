// Kolmogorov-Smirnov: https://www.itl.nist.gov/div898/handbook/eda/section3/eda35g.htm
// Kolmogorov-Smirnov Implementation: https://stackoverflow.com/questions/37625280/kolmogorov-smrinov-test-in-c

#include <iostream>
#include <algorithm>

double ks(double num[], int length) {
	double tmp[length];
	std::copy(num, num + length, tmp);
	std::sort(tmp, tmp + length); // Must sort data
	double d[length], n = (double) length, maxPlus, maxNeg; // d stores differences between ECDF and uniform distribution CDF

	for (int i = 0; i < length; ++i) { // Compute maximum distance D+ when uniform distribution is above EDF
		d[i] = ((i + 1) / n) - tmp[i]; // Is this correct? ECDF - uniform distribution CDF?
	}
	maxPlus = *std::max_element(d, d + length);

	for (int i = 0; i < length; ++i) { // Compute maximum distance D- when uniform distribution is below EDF
		d[i] = (tmp[i] - (i) / n); // Is this correct? uniform distribution CDF - ECDF?
	}
	maxNeg = *std::max_element(d, d + length);
	
	return std::max(maxPlus, maxNeg); // Return D
}
