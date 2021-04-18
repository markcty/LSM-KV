#include "kvstore.h"
#include "utils.h"

KVStore::KVStore(const string &dir) : KVStoreAPI(dir), timeStamp(0) {
  ios_base::sync_with_stdio(false); // turn off sync to accelerate
}

void KVStore::put(uint64_t key, const string &value) {
  memTable.put(key, value);
  if (overflow(memTable.getSize(), memTable.getLength())) {
    if (!utils::dirExists(dir + "/level-0")) utils::mkdir((dir + "/level-0").c_str());

    vector<string> existsFiles;
    utils::scanDir(dir + "/level-0", existsFiles);

    SSTable::toSSTable(memTable, dir + "/level-0/" + to_string(existsFiles.size()) + ".sst", ++timeStamp);

    memTable.reset();

    compaction(0);
  }
}

string KVStore::get(uint64_t key) { return memTable.get(key); }

bool KVStore::del(uint64_t key) { return memTable.remove(key); }

/*
 * This resets the kvstore. All key-value pairs should be removed,
 * including memtable and all sstables files.
 */
void KVStore::reset() {
  timeStamp = 0;
}

bool KVStore::overflow(unsigned long size, unsigned long length) {
  auto offSetSize = length * 4;
  return HeaderSize + BloomFilterSize + size + offSetSize > SSTableSize;
}

void KVStore::compaction(int level) {
  string dirName = "/level-" + to_string(level);

  if (!utils::dirExists(dirName)) return;

  vector<string> dirFiles;
  int maxFiles = pow2(level + 1), files = utils::scanDir(dirName, dirFiles);
  if (files <= maxFiles) return;

  // get the files which need compaction
  vector<string> compactionFiles;
  if (level == 0) compactionFiles = dirFiles;
  else {
    // find the files with smallest timeStamp
    vector<pair<uint64_t, int>> timeStamps(dirFiles.size());
    for (int i = 0; i < dirFiles.size(); i++)
      timeStamps.emplace_back(SSTable::readTimeStamp(dirFiles[i]), i);

    sort(timeStamps.begin(),
         timeStamps.end(),
         [](pair<uint64_t, int> x, pair<uint64_t, int> y) { return x.first < y.first; });

    for (int i = 0; i < files - maxFiles; i++)
      compactionFiles.push_back(dirFiles[timeStamps[i].second]);
  }

  const auto k = compactionFiles.size();
  unsigned long pairs = 0;

  // read dictionaries from dics
  vector<vector<pair<uint64_t, string>>> dics(compactionFiles.size());
  for (int i = 0; i < k; i++) {
    SSTable::readDic(compactionFiles[i], dics[i]);
    pairs += dics[i].size();
  }

  string fileName;

  // merge the dics
  // There are better algorithms, but since k is usually very small, and for simplicity,
  // I use the naive solution O(pairs * k)
  vector<int> indexes(k);
  vector<pair<uint64_t, const string *>> merged;
  unsigned long mergedSize = 0; // key and value size in merged
  while (pairs) {
    uint64_t minKey = UINT64_MAX, minIndex = 0;
    for (int i = 0; i < k; i++) {
      if (indexes[i] < dics[i].size() && dics[i][indexes[i]].first < minKey) {
        minKey = dics[i][indexes[i]].first;
        minIndex = i;
      }
    }
    const string *minValue = &dics[minIndex][indexes[minIndex]++].second;
    merged.emplace_back(minKey, minValue);
    mergedSize += sizeof(minKey) + minValue->size();
    if (overflow(mergedSize, merged.size())) {
      SSTable::toSSTable(merged, fileName, ++timeStamp);
    }
    pairs--;
  }

  compaction(level + 1);
}

int KVStore::pow2(int n) {
  int ret = 1;
  for (int i = 0; i < n; i++) ret *= 2;
  return ret;
}

KVStore::~KVStore() = default;

