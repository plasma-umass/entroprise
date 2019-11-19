#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <unordered_map>

using namespace std;

int main(int argc, char *argv[]) {
	if (argc != 3) {
		cerr << "usage: [NOBJS] [SIZE]" << endl;
		return -1;
	}

	const int NOBJS = stoi(argv[1]), SZ = stoi(argv[2]);
	char **objs = new char *[NOBJS];

	for (int i = 0; i < NOBJS; ++i)
		objs[i] = (char *) malloc(SZ);
	for (int i = 0; i < NOBJS; ++i)
		free(objs[i]);

	delete objs[NOBJS];

	srand(time(NULL));
	int next = 0, r = rand();
	unordered_multimap<char *, int> counter;
	unordered_multimap<char *, int>::iterator it;

	for (int i = 0; i < NOBJS; ++i) {
		char *ptr = (char *) malloc(SZ);
		it = counter.find(ptr);
	  	if (it == counter.end()) // If not in map,
			counter.insert(pair<char *, int>(ptr, ++next)); // Then insert it with unique value
		else // If in map,
			counter.insert(pair<char *, int>(ptr, it->second)); // Then duplicate it
		free(ptr);
	}

	for (it = counter.begin(); it != counter.end(); ++it) {
		unsigned int cur = it->second ^ r;
		fwrite(&cur, sizeof(cur), 1, stdout);
	}

	return 0;
}
