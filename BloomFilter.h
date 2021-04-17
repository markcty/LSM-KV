#ifndef LSM_KV__BLOOMFILTER_H_
#define LSM_KV__BLOOMFILTER_H_

#include <bitset>
#include <fstream>
#include "SkipList.h"

using namespace std;

class BloomFilter {
 private:
  bitset<10240> bits;
 public:
  explicit BloomFilter(const SkipList &memTable);
  void write(ofstream &out);
};

#endif //LSM_KV__BLOOMFILTER_H_
