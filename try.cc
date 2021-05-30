#include <iostream>

#include "kvstore.h"

using namespace std;

int main() {
  KVStore store("./data");
  for (uint64_t i = 0; i < 80000000; i++) store.put(i, to_string(i));
  return 0;
}