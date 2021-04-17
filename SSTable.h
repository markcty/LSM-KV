#ifndef LSM_KV__SSTABLE_H_
#define LSM_KV__SSTABLE_H_

#include <fstream>

#include "SkipList.h"
#include "BloomFilter.h"

class SSTable {
 public:
  /**
   * convert a mem table to a SSTable file
   * @param memTable mem table
   * @param fileName file name
  */
  static void toSSTable(const SkipList &memTable, const string &fileName, uint64_t timeStamp);
};

#endif //LSM_KV__SSTABLE_H_
