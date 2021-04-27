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
  if (value == "~DELETED~") return "";
  if (!value.empty()) return value;
  int level = 0;
  auto dirName = storagePath + "/level-0";
  while (utils::dirExists(dirName)) {
    vector<string> dirFiles;
    utils::scanDir(dirName, dirFiles);
    for (string &name:dirFiles) name.insert(0, dirName + '/');

    string ans;
    auto maxTime = 0;
    for (const auto &file:dirFiles) {
      // optimize
      const auto &header = SSTable::readHeader(file);
      if (!(header.minKey <= key && key <= header.maxKey)) continue;

      value = SSTable::get(file, key);
      if (value.empty()) continue;

      if (header.timeStamp > maxTime) ans = value;
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

void KVStore::reset() {
  timeStamp = 0;

  memTable.reset(); // reset memTable

  // clean all SSTables
  string dir = storagePath + "/level-0";
  vector<string> files;
  int level = 0;
  while (utils::scanDir(dir, files) > 0) {
    for (const auto &file:files)
      utils::rmfile((dir + "/" + file).c_str()); // NOLINT(performance-inefficient-string-concatenation)
    dir = storagePath + "/level-" + to_string(++level);
  }
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
      return (lHeader.timeStamp > rHeader.timeStamp ||
          (lHeader.timeStamp == rHeader.timeStamp && lHeader.minKey > rHeader.minKey));
    };
    priority_queue<nameHeader, vector<nameHeader>, decltype(cmp)> q(cmp);
    int k = fileNum - maxFileNum;
    for (const auto &file:dirFiles) q.emplace(file, SSTable::readHeader(file));
    while (k--) {
      auto top = q.top();
      q.pop();
      compactionFiles.push_back(top.first);
      compactionHeaders.push_back(top.second);
    }
  }

  uint64_t minKey = UINT64_MAX, maxKey = 0, maxTimeStamp = 0;
  for (const auto &header:compactionHeaders) {
    maxKey = max(header.maxKey, maxKey);
    minKey = min(header.minKey, minKey);
    maxTimeStamp = max(header.timeStamp, maxTimeStamp);
  }

  // get SSTables in next level whose keys intersects with compaction Files
  string nextDir = storagePath + "/level-" + to_string(level + 1);
  if (!utils::dirExists(nextDir)) utils::mkdir(nextDir.c_str());
  dirFiles.clear();
  utils::scanDir(nextDir, dirFiles);
  for (string &name:dirFiles) name.insert(0, nextDir + '/');
  for (const auto &file:dirFiles) {
    auto header = SSTable::readHeader(file);
    if ((minKey <= header.minKey && header.minKey <= maxKey)
        || (minKey <= header.maxKey && header.maxKey <= maxKey)) {
      compactionFiles.push_back(file);
      compactionHeaders.push_back(header);
      maxTimeStamp = max(maxTimeStamp, header.timeStamp);
    }
  }

  const auto k = compactionFiles.size();
  unsigned long pairNum = 0;

  // check if it is the last level
  bool lastLevel = false;
  if (!utils::dirExists(storagePath + "/level1-" + to_string(level + 2))) lastLevel = true;


  // read dictionaries from dics
  vector<vector<pair<uint64_t, string>>> dics(compactionFiles.size());
  for (int i = 0; i < k; i++) {
    SSTable::readDic(compactionFiles[i], dics[i]);
    utils::rmfile(compactionFiles[i].c_str());
    pairNum += dics[i].size();
  }

  // merge the dics, convert to SSTables
  vector<int> indices(k);
  SSTableDic mergedDic;
  unsigned long mergedSize = 0; // key and value size in mergedDic used to check overflow
  auto nameIndex = 0;
  uint64_t lastTimeStamp = 0;
  while (pairNum--) {
    minKey = UINT64_MAX;
    auto minDic = 0;
    for (int i = 0; i < k; i++) {
      if (indices[i] < dics[i].size() && dics[i].at(indices[i]).first < minKey) {
        minKey = dics[i].at(indices[i]).first;
        minDic = i;
      }
    }
    const auto &pair = dics[minDic].at(indices[minDic]++);
    if (overflow(mergedSize + 8 + pair.second.size(), mergedDic.size() + 1)) {
      SSTable::toSSTable(mergedDic,
                         nextDir + "/" + to_string(maxTimeStamp) + to_string(nameIndex++),
                         maxTimeStamp);
      mergedSize = 0;
      mergedDic.clear();
    }
    if (!mergedDic.empty() && pair.first == mergedDic.back().first
        && compactionHeaders[minDic].timeStamp < lastTimeStamp)
      continue;  // duplicate keys, select the one with the largest timeStamp
    if (lastLevel && pair.second == "~DELETED~") continue; // last level should not contain deleted keys
    mergedDic.emplace_back(pair.first, pair.second);
    lastTimeStamp = minDic;
    mergedSize += 8 + pair.second.size();
  }

  // convert the remaining key value pairs to a SSTable
  if (mergedSize > 0) {
    SSTable::toSSTable(mergedDic,
                       nextDir + "/" + to_string(maxTimeStamp) + to_string(nameIndex++),
                       maxTimeStamp);
  }
  compaction(level + 1);
}

int KVStore::pow2(int n) {
  int ret = 1;
  for (int i = 0; i < n; i++) ret *= 2;
  return ret;
}

KVStore::~KVStore() = default;

