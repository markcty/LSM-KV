#include "kvstore.h"
#include "utils.h"

KVStore::KVStore(const string &_storagePath) :
    KVStoreAPI(_storagePath), timeStamp(0), storagePath(_storagePath) {
  ios_base::sync_with_stdio(false); // turn off sync to accelerate
}

void KVStore::put(uint64_t key, const string &value) {
  // if overflow, convert memTable to a SSTable and do compaction
  if (overflow(memTable.getSize() + 8 + value.size(), memTable.getLength() + 1)) {
    if (!utils::dirExists(storagePath + "/level-0")) utils::mkdir((storagePath + "/level-0").c_str());

    vector<string> existsFiles;
    utils::scanDir(storagePath + "/level-0", existsFiles);

    SSTable::toSSTable(memTable, storagePath + "/level-0/" + to_string(timeStamp) + ".sst", timeStamp);
    timeStamp++;

    memTable.reset();

    compaction(0);
  }
  memTable.put(key, value);
}

string KVStore::get(uint64_t key) {
  auto value = memTable.get(key);
  if (!value.empty()) return value;
  int level = 0;
  auto dirName = storagePath + "/level-0";
  while (utils::dirExists(dirName)) {
    vector<string> dirFiles;
    utils::scanDir(dirName, dirFiles);
    for (string &name:dirFiles) name.insert(0, dirName + '/');

    string ans;
    auto minTime = UINT64_MAX;
    for (const auto &file:dirFiles) {
      vector<pair<uint64_t, string>> dic;
      SSTable::readDic(file, dic);
      auto time = SSTable::readHeader(file).timeStamp;
      for (const auto &pair:dic)
        if (pair.first == key && time < minTime) {
          minTime = time;
          ans = pair.second;
        }
    }
    if (!ans.empty()) return ans;
    dirName = storagePath + "/level-" + to_string(++level);
  }
  return "";
}

bool KVStore::del(uint64_t key) {
  auto value = get(key);
  if (value.empty()) return false;
  memTable.remove(key);
  memTable.put(key, "~DELETED~");
  return true;
}

/*
 * This resets the kvstore. All key-value pairs should be removed,
 * including memtable and all sstables files.
 */
void KVStore::reset() {
  timeStamp = 0;
}

bool KVStore::overflow(unsigned long size, unsigned long length) {
  auto offSetSize = length * 4;
  return HeaderSize + BloomFilterSize + size + offSetSize >= SSTableSize;
}

void KVStore::compaction(int level) {
  string dir = storagePath + "/level-" + to_string(level);

  if (!utils::dirExists(dir)) return;

  vector<string> dirFiles;
  int maxFileNum = pow2(level + 1), fileNum = utils::scanDir(dir, dirFiles);
  if (fileNum <= maxFileNum) return;

  for (string &name:dirFiles) name.insert(0, dir + '/');

  // get the files which need compaction
  vector<string> compactionFiles;
  vector<SSTableHeader> compactionHeaders;
  if (level == 0) {
    compactionFiles = dirFiles;
    for (auto const &file:dirFiles) compactionHeaders.emplace_back(SSTable::readHeader(file));
  } else {
    // find the files with smallest timeStamp
    using nameHeader = pair<string, SSTableHeader>;
    auto cmp = [](const nameHeader &left, const nameHeader &right) {
      const auto &lHeader = left.second;
      const auto &rHeader = right.second;
      return (lHeader.timeStamp < rHeader.timeStamp ||
          (lHeader.timeStamp == rHeader.timeStamp && lHeader.minKey > rHeader.minKey));
    };
    priority_queue<nameHeader, vector<nameHeader>, decltype(cmp)> q(cmp);
    int k = fileNum - maxFileNum;
    while (k--) {
      auto top = q.top();
      q.pop();
      compactionFiles.push_back(top.first);
      compactionHeaders.push_back(top.second);
    }
  }

  uint64_t minDic = UINT64_MAX, maxKey = 0, maxTimeStamp = 0;
  for (const auto &header:compactionHeaders) {
    maxKey = max(header.maxKey, maxKey);
    minDic = min(header.minKey, minDic);
    maxTimeStamp = max(header.timeStamp, maxTimeStamp);
  }

  // get SSTables in next level whose keys intersects with compaction Files
  string nextDir = storagePath + "/level-" + to_string(level + 1);
  if (!utils::dirExists(nextDir)) utils::mkdir(nextDir.c_str());
  dirFiles.clear();
  utils::scanDir(nextDir, dirFiles);
  for (const auto &file:dirFiles) {
    auto header = SSTable::readHeader(file);
    if ((minDic <= header.minKey && header.minKey <= maxKey)
        || (minDic <= header.maxKey && header.maxKey <= maxKey)) {
      compactionFiles.push_back(file);
      maxTimeStamp = max(maxTimeStamp, header.timeStamp);
    }
  }

  const auto k = compactionFiles.size();
  unsigned long pairNum = 0;

  // read dictionaries from dics
  vector<vector<pair<uint64_t, string>>> dics(compactionFiles.size());
  for (int i = 0; i < k; i++) {
    SSTable::readDic(compactionFiles[i], dics[i]);
    utils::rmfile(compactionFiles[i].c_str());
    pairNum += dics[i].size();
  }

  // merge the dics, convert to SSTables
  // There are better algorithms, but since k is usually very small, and for simplicity,
  // I use the naive solution O(pairNum * k)
  vector<int> indices(k);
  SSTableDic mergedDic;
  unsigned long mergedSize = 0; // key and value size in mergedDic used to check overflow
  auto nameIndex = 0;
  while (pairNum--) {
    minDic = UINT64_MAX;
    auto minIndex = 0;
    for (int i = 0; i < k; i++) {
      if (indices[i] < dics[i].size() && dics[i].at(indices[i]).first < minDic) {
        minDic = dics[i].at(indices[i]).first;
        minIndex = i;
      }
    }
    const auto &pair = dics[minIndex].at(indices[minIndex]);
    if (overflow(mergedSize + 8 + pair.second.size(), mergedDic.size() + 1)) {
      SSTable::toSSTable(mergedDic,
                         nextDir + "/" + to_string(maxTimeStamp) + to_string(nameIndex++),
                         maxTimeStamp);
      mergedSize = 0;
      mergedDic.clear();
    }
    mergedDic.emplace_back(pair.first, pair.second);
  }

  compaction(level + 1);
}

int KVStore::pow2(int n) {
  int ret = 1;
  for (int i = 0; i < n; i++) ret *= 2;
  return ret;
}

//void KVStore::compaction0() {
//  string dir = storagePath + "/level-0";
//  vector<string> dirFiles;
//  auto fileNum = utils::scanDir(dir, dirFiles);
//  if (fileNum <= 2) return;
//
//  // get key range in leve l0
//  uint64_t minDic = UINT64_MAX, maxKey = 0, maxTimeStamp = 0;
//  for (const auto &file:dirFiles) {
//    auto header = SSTable::readHeader(file);
//    maxKey = max(header.maxKey, maxKey);
//    minDic = min(header.minKey, minDic);
//    maxTimeStamp = max(header.timeStamp, maxTimeStamp);
//  }
//
//  // get SSTables in level 1 involved in compaction
//  string nextDir = storagePath + "/level-1";
//  if (!utils::dirExists(nextDir)) utils::mkdir(nextDir.c_str());
//  vector<string> compactionFiles(dirFiles); // put all files in level 0 in files need compactions
//  dirFiles.clear();
//  for (const auto &file:dirFiles) {
//    auto header = SSTable::readHeader(file);
//    if ((minDic <= header.minKey && header.minKey <= maxKey)
//        || (minDic <= header.maxKey && header.maxKey <= maxKey)) {
//      compactionFiles.push_back(file);
//      maxTimeStamp = max(maxTimeStamp, header.timeStamp);
//    }
//  }
//
//  // read all key value pairs from SSTables
//  auto k = compactionFiles.size();
//  unsigned long pairNum = 0;
//  vector<SSTableDic> dics;
//  dics.reserve(k);
//  for (int i = 0; i < k; i++) {
//    SSTable::readDic(compactionFiles[i], dics[i]);
//    pairNum += dics[i].size();
//  }
//
//  // merge all key value pairs, convert to SSTables
//  SSTableDic mergedDic;
//  unsigned long mergedSize = 0;
//  int nameIndex = 0;
//  vector<int> indices(k);
//  while (pairNum--) {
//    minDic = UINT64_MAX;
//    auto minIndex = 0;
//    for (int i = 0; i < k; i++) {
//      if (indices[i] < dics[i].size() && dics[i].at(indices[i]).first < minDic) {
//        minDic = dics[i].at(indices[i]).first;
//        minIndex = i;
//      }
//    }
//    const auto &pair = dics[minIndex].at(indices[minIndex]);
//    if (overflow(mergedSize + 8 + pair.second.size(), mergedDic.size() + 1)) {
//      SSTable::toSSTable(mergedDic, nextDir + "/" + to_string(maxTimeStamp) + to_string(nameIndex++), maxTimeStamp);
//      mergedSize = 0;
//      mergedDic.clear();
//    }
//    mergedDic.emplace_back(pair.first, pair.second);
//  }
//
//  compaction(1);
//}

KVStore::~KVStore() = default;

