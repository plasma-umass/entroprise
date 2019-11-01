#include <iostream>
#include <unordered_map>
#include <cmath>
#include <cstdlib>
#include <future>
#include <iomanip>

using namespace std;

float run(const int OBJECT_SIZE, const int MIN_ALLOC) {
	char ** objs = new char *[MIN_ALLOC];
  	unordered_map<char *, int> counter;
	
	for (int i = 0; i < MIN_ALLOC; i++)
		objs[i] = (char *) malloc(OBJECT_SIZE);
  	for (int i = 0; i < MIN_ALLOC; i++)
		free(objs[i]);

  	for (int i = 0; i < MIN_ALLOC; i++) {
	  char *p = (char *) malloc(OBJECT_SIZE);
	  free(p);
	  if (counter.find(p) == counter.end())
	    counter[p] = 0;
	  counter[p]++;
  	}

  	float entropy = 0.0;
  	for (auto it = counter.begin(); it != counter.end(); ++it) {
		char *address = (*it).first;
  	  	int count = (*it).second;
  	  	entropy += -log(count / (double) MIN_ALLOC) / log(2.0) * (count / (double) MIN_ALLOC);
  	}
	delete [] objs;
	return entropy;
}

int main(int argc, char *argv[]) {
	const int OBJECT_SIZE = stoi(argv[1]), MIN_ALLOC = stoi(argv[2]), NTHREADS = stoi(argv[3]);
	future<float> *threads = new future<float>[NTHREADS];
	float entropy = 1, max = log(MIN_ALLOC) / log(2.0);

	for (int i = 0; i < NTHREADS; i++) // Run all threads
		threads[i] = async(run, OBJECT_SIZE, MIN_ALLOC);
	for (int i = 0; i < NTHREADS; i++) // Compute average
		entropy = entropy * threads[i].get();

	entropy = pow(entropy, 1.0 / NTHREADS); // Compute average 
	cout << fixed;
	cout << setprecision(4); // For alignment
	cout << entropy << "   " << max << "   " << entropy * 100.0 / max << "%" << endl;

  	return 0;
}
