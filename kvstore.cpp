//
// Created by markcty on 2021/4/14.
//

#include "kvstore.h"
#include <string>

KVStore::KVStore(const string &dir) : KVStoreAPI(dir) {}

void KVStore::put(uint64_t key, const string &value) {
  memTable.put(key, value);
  if (memOverflow()) {
    if (!dirExists(dir + "/level-0")) mkdir((dir + "/level-0").c_str());
    SSTable::toSSTable(memTable, dir + "/level-0/0.sst", 0);
  }
}

string KVStore::get(uint64_t key) { return memTable.get(key); }

bool KVStore::del(uint64_t key) { return memTable.remove(key); }

/**
 * This resets the kvstore. All key-value pairs should be removed,
 * including memtable and all sstables files.
 */
void KVStore::reset() {
}

bool KVStore::memOverflow() {
  auto size = memTable.getSize();
  auto length = memTable.getLength();
  auto offSetSize = length * 4;
  return HeaderSize + BloomFilterSize + size + offSetSize > SSTableSize;
}

