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
    **/
  static void toSSTable(const SkipList &memTable, const string &fileName, uint64_t timeStamp);

  /**
    * convert a mem table to a SSTable file
    * @param memTable mem table
    * @param fileName file name
    **/
  static void toSSTable(const vector<pair<uint64_t, const string *>> &dic, const string &fileName, uint64_t timeStamp);

  /**
   * read the timeStamp from a SSTable file
   * @param fileName file name
   * @return timeStamp of SSTable
   **/
  static uint64_t readTimeStamp(const string &fileName);

  /**
    * read the timeStamp from a SSTable file
    * @param fileName file name
    * @param dic reference to dic
    */
  static void readDic(const string &fileName, vector<pair<uint64_t, string>> &dic);
};

#endif //LSM_KV__SSTABLE_H_
