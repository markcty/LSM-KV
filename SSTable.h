#ifndef LSM_KV__SSTABLE_H_
#define LSM_KV__SSTABLE_H_

#include <algorithm>
#include <bitset>
#include <fstream>

#include "SkipList.h"

// Its ascending attribute must be promised by code
using SSTableDic = vector<pair<uint64_t, string>>;

using SSTableIndex = pair<uint64_t, int>;

class SSTableHeader {
 public:
  uint64_t timeStamp{}, length{}, minKey{}, maxKey{};
  SSTableHeader();
  explicit SSTableHeader(uint64_t _timeStamp, uint64_t _size, uint64_t _minKey,
                         uint64_t _maxKey);
  SSTableHeader(const SSTableHeader &other);
};

class BloomFilter {
 private:
  bitset<10240 * 8> bits;

 public:
  explicit BloomFilter(const SkipList &memTable);
  explicit BloomFilter(const SSTableDic &dic);
  explicit BloomFilter(ifstream &in);
  BloomFilter();
  void write(ofstream &out);
};

class SSTableCache {
 private:
  SSTableHeader header;
  vector<SSTableIndex> index;
  BloomFilter bloomFilter;
  const string fileName;

 public:
  explicit SSTableCache(const SkipList &memTable, uint64_t timeStamp,
                        string _fileName);
  explicit SSTableCache(const SSTableDic &dic, uint64_t timeStamp,
                        string _fileName);
  explicit SSTableCache(string _fileName);
  string get(uint64_t key) const;
  SSTableHeader getHeader() const;
};

class SSTable {
 public:
  // utils
  static void write64(ofstream &out, uint64_t n);
  static void write32(ofstream &out, uint32_t n);
  static void read64(ifstream &in, uint64_t &n);
  static void read32(ifstream &in, int &n);

  /**
   * convert a mem table to a SSTable file
   * @param memTable mem table
   * @param fileName file name
   */
  static void toSSTable(const SkipList &memTable, const string &fileName,
                        uint64_t timeStamp);

  /**
   * convert a mem table to a SSTable file
   * @param memTable mem table
   * @param fileName file name
   */
  static void toSSTable(const SSTableDic &dic, const string &fileName,
                        uint64_t timeStamp);

  /**
   * read the timeStamp from a SSTable file
   * @param fileName file name
   * @return timeStamp of SSTable
   */
  static SSTableHeader readHeader(const string &fileName);

  /**
   * read dictionary from a SSTable file
   * @param fileName file name
   * @param dic reference to dic
   */
  static void readDic(const string &fileName, SSTableDic &dic);

  /**
   * get value from a SSTable file
   * @param fileName file name
   * @param key key
   * @return value, empty string if not found
   */
  static string get(const string &fileName, uint64_t key);
};

#endif  // LSM_KV__SSTABLE_H_
