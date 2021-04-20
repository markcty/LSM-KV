#ifndef LSM_KV__BLOOMFILTER_H_
#define LSM_KV__BLOOMFILTER_H_

#include <bitset>
#include <fstream>
#include "SkipList.h"

using namespace std;

class BloomFilter {
 private:
  bitset<10240 * 8> bits;
 public:
  explicit BloomFilter(const SkipList &memTable);
  explicit BloomFilter(const vector<pair<uint64_t, const string *>> &dic);
  explicit BloomFilter(ifstream &in);
  void write(ofstream &out);
};

#endif //LSM_KV__BLOOMFILTER_H_
