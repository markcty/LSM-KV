#include <iostream>
#include <vector>
#include "kvstore.h"
#include <cstdlib>
using namespace std;

int main() {
  srand(time(0));
  KVStore store("../data");
  vector<uint64_t> keys;
  for (uint64_t i = 0; i < 800000; i++) {
    auto key = rand();
    keys.push_back(key);
    store.put(key, to_string(key));
  }
  cout << keys[5] << " " << store.get(keys[5]) << endl;
  cout << keys[9] << "     " << store.get(keys[9]) << endl;
  cout << keys[20] << " " << store.get(keys[20]) << endl;
  cout << keys[718283] << " " << store.get(keys[718283]) << endl;
  cout << keys[192874] << " " << store.get(keys[192874]) << endl;
//  cout << store.get(2832015) << endl;
//  cout << store.get(4832015) << endl;

  return 0;
}