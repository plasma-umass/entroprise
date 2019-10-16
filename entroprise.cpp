#include <iostream>
#include <unordered_map>
using namespace std;

const auto OBJECT_SIZE = 16384; // 4096;
const auto MIN_ALLOC = 50000;

class Object {
  char foo[OBJECT_SIZE];
};

int
main()
{
  Object ** objs = new Object *[MIN_ALLOC];
  // Allocate and then free MIN_ALLOC objects.
  for (auto i = 0; i < MIN_ALLOC; i++) {
    objs[i] = new Object;
  }
  for (auto i = 0; i < MIN_ALLOC; i++) {
    delete objs[i];
  }

  unordered_map<Object *, int> counter;
  // Now repeatedly allocate and free one object.
  for (auto i = 0; i < MIN_ALLOC; i++) {
    Object * p;
    //    p = (Object *) (0x10000 * (i % 8));
    p = new Object;//    auto p = (Object *) 0x10000; // new Object;
    if (counter.find(p) == counter.end()) {
      counter[p] = 0;
    }
    counter[p]++;
    //    cout << reinterpret_cast<void *>(p) << endl;
    delete p;
  }

  // Print out the histogram.
  auto entropy = 0.0;
  for (auto it = counter.begin(); it != counter.end(); ++it) {
    auto address = (*it).first;
    auto count = (*it).second;
    entropy += -log(count/(double)MIN_ALLOC)/log(2.0) * (count/(double)MIN_ALLOC);
    // cout << address << ", " << count << endl;
  }
  //  entropy = -entropy;
  const auto maxEntropy = log(MIN_ALLOC)/log(2.0);
  cout << "entropy  = " << entropy << endl;
  cout << "maximum entropy = " << maxEntropy << endl;
  cout << "percent of max achievable = " << entropy * 100.0 / maxEntropy << " %" << endl;
  return 0;
}
