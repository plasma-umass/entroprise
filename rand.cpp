#include <iostream>

using namespace std;

int main(int argc, char *argv[]) {
	if (argc != 2) {
		cerr << "usage: [NRANDS]" << endl;
		return -1;
	}

	const int NRANDS = stoi(argv[1]);
	unsigned int *n = new unsigned int[NRANDS];
	srand(time(NULL));

	for (int i = 0; i < NRANDS; ++i) {
		n[i] = (unsigned int) rand();
	  	n[i] /= 1024;
	  	n[i] *= 1024;
	}

	fwrite(n, sizeof(n[0]), NRANDS, stdout);
	return 0;
}
