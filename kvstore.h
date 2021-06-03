#ifndef LSM_KV__KVSTORE_H_
#define LSM_KV__KVSTORE_H_

#include <assert.h>
#include <unistd.h>

#include <algorithm>
#include <iostream>
#include <map>
#include <queue>
#include <string>

#include "SSTable.h"
#include "SkipList.h"
#include "kvstore_api.h"
#include "utils.h"

using namespace std;

class KVStore : public KVStoreAPI {
 private:
  string storagePath;

  uint64_t timeStamp;

  int fileNums;

  SkipList memTable;

  map<string, SSTableCache *> cache;

  // if memTable is large enough to be converted to a SSTable
  static inline bool overflow(unsigned long length, unsigned long valueSize);

  void compaction(int level);

  // utils
  // return 2^n
  static int pow2(int n);

  // extract level from a path
  static int getLevel(const string &path);

 public:
  explicit KVStore(const string &_storagePath);

  virtual ~KVStore();

  void put(uint64_t key, const string &value) override;

  string get(uint64_t key) override;

  bool del(uint64_t key) override;

  void reset() override;
};

#endif  // LSM_KV__KVSTORE_H_
