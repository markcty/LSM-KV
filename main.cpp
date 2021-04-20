#include <iostream>
#include <vector>
#include "kvstore.h"
#include "SSTable.h"

using namespace std;

int main() {
  KVStore store("../data");
  for (uint64_t i = 0; i < 800000; i++) {
    store.put(i, to_string(i));
  }
  cout << store.get(3) << endl;
  cout << store.get(5) << endl;
  cout << store.get(119294) << endl;
  cout << store.get(750094) << endl;

  return 0;
}