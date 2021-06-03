#include <iostream>

#include "kvstore.h"

using namespace std;

const auto path = "./data";

int main() {
  KVStore* store = new KVStore(path);
  store->put(1, "1");
  store->put(2, "2");
  for (int i = 0; i <= 102400; ++i) store->put(i + 30, std::string(1024, 'x'));
  cout << store->get(1) << endl;
  cout << store->get(2) << endl;
  delete store;

  store = new KVStore(path);
  cout << store->get(1) << endl;
  cout << store->get(2) << endl;
  store->put(1, "3");
  store->put(2, "4");
  for (int i = 0; i <= 10240; ++i) store->put(i + 30, std::string(1024, 'x'));
  cout << store->get(1) << endl;
  cout << store->get(2) << endl;
  store->reset();
  delete store;
  return 0;
}