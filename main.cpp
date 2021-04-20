#include <iostream>
#include <vector>
#include "kvstore.h"
#include "SSTable.h"

using namespace std;

int main() {
  KVStore store("../data");
  for (uint64_t i = 0; i < 8000000; i++) store.put(i, to_string(i));

  vector<pair<uint64_t, string>> dic;
  cout << dic.size() << endl;

  return 0;
}