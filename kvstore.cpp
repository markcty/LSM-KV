#include "kvstore.h"
#include "utils.h"

KVStore::KVStore(const string &_storagePath) :
    KVStoreAPI(_storagePath), timeStamp(0), storagePath(_storagePath), fileNums(0) {
  ios_base::sync_with_stdio(false); // turn off sync to accelerate
  // build cache
  string dir = storagePath + "/level-0";
  int level = 0;
  while (utils::dirExists(dir)) {
    vector<string> files;
    utils::scanDir(dir, files);
    for (const auto &file:files) {
      auto fileName = dir + "/" + file;// NOLINT(performance-inefficient-string-concatenation)}
      cache.emplace(fileName, new SSTableCache(fileName));

      // check file numbers to avoid duplicate file names
      fileNums = max(stoi(file) + 1, fileNums);
    }
    dir = storagePath + "/level-" + to_string(++level);
  }
}

void KVStore::put(uint64_t key, const string &value) {
  // if overflow, convert memTable to a SSTable and do compaction
  if (overflow(memTable.getLength() + 1, memTable.getValueSize() + value.size())) {
    if (!utils::dirExists(storagePath + "/level-0")) utils::mkdir((storagePath + "/level-0").c_str());

    auto fileName = storagePath + "/level-0/" + to_string(fileNums++);
    SSTable::toSSTable(memTable, fileName, timeStamp);
    cache.emplace(fileName, new SSTableCache(memTable, timeStamp, fileName));
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

  auto i = cache.cbegin();

  int level = 0;
  while (i != cache.cend()) {
    string ans;
    uint64_t maxTime = 0;
    while (i != cache.cend() && getLevel(i->first) == level) {
      value = i->second->get(key);
      if (!value.empty() && i->second->getHeader().timeStamp >= maxTime) {
        ans = value;
        maxTime = i->second->getHeader().timeStamp;
      }
      i++;
    }
    if (ans == "~DELETED~") return "";
    if (!ans.empty()) return ans;
    level++;
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
  fileNums = 0;

  memTable.reset(); // reset memTable

  // clean all SSTables
  string dir = storagePath + "/level-0";
  vector<string> files;
  int level = 0;
  while (utils::dirExists(dir) && utils::scanDir(dir, files) > 0) {
    for (const auto &file:files)
      utils::rmfile((dir + "/" + file).c_str()); // NOLINT(performance-inefficient-string-concatenation)
    dir = storagePath + "/level-" + to_string(++level);
    files.clear();
  }
}

bool KVStore::overflow(unsigned long length, unsigned long valueSize) {
  auto indexSize = (length + 1) * 12;
  return 32 + 10240 + indexSize + valueSize >= 2 * 1024 * 1024;
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
  if (level == 0) {
    compactionFiles = dirFiles;
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
    for (const auto &file:dirFiles) q.emplace(file, cache.at(file)->getHeader());
    while (k--) {
      auto top = q.top();
      q.pop();
      compactionFiles.push_back(top.first);
    }
  }

  // prepare minKey, maxKey, maxTimeStamp for overlap detection
  uint64_t minKey = UINT64_MAX, maxKey = 0, maxTimeStamp = 0;
  for (const auto &file:compactionFiles) {
    const auto &header = cache.at(file)->getHeader();
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
    const auto &header = cache.at(file)->getHeader();
    if ((minKey <= header.minKey && header.minKey <= maxKey)
        || (minKey <= header.maxKey && header.maxKey <= maxKey)) {
      compactionFiles.push_back(file);
      maxTimeStamp = max(maxTimeStamp, header.timeStamp);
    }
  }

  const auto k = compactionFiles.size();
  unsigned long pairNum = 0;

  // check if it is the last level
  bool lastLevel = false;
  if (!utils::dirExists(storagePath + "/level-" + to_string(level + 2))) lastLevel = true;

  // read dictionaries from dics
  vector<SSTableDic> dics(compactionFiles.size());
  for (int i = 0; i < k; i++) {
    // read dic
    SSTable::readDic(compactionFiles[i], dics[i]);
    utils::rmfile(compactionFiles[i].c_str());
    pairNum += dics[i].size();
  }

  // merge the dics, convert to SSTables
  vector<int> indices(k);
  SSTableDic mergedDic;
  uint64_t lastTimeStamp = 0;
  while (pairNum--) {
    // find the next pair to merge
    minKey = UINT64_MAX;
    auto minDic = 0;
    for (int i = 0; i < k; i++) {
      if (indices[i] < dics[i].size() && dics[i].at(indices[i]).first < minKey) {
        minKey = dics[i].at(indices[i]).first;
        minDic = i;
      }
    }
    const auto &pair = dics[minDic].at(indices[minDic]++);

    const auto &header = cache.at(compactionFiles[minDic])->getHeader();

    // duplicate keys, select the one with the largest timeStamp
    if (!mergedDic.empty() && pair.first == mergedDic.back().first) {
      if (header.timeStamp < lastTimeStamp)
        continue;
      else
        mergedDic.pop_back();
    }

    mergedDic.emplace_back(pair.first, pair.second);
    lastTimeStamp = header.timeStamp;
  }

  // split to SSTable
  SSTableDic t; // TODO: can be optimized out
  uint64_t valueSize = 0, length = 0;
  for (const auto &pair:mergedDic) {
    // last level should not contain deleted value
    if (lastLevel && pair.second == "~DELETED~") continue;

    if (overflow(length + 1, valueSize + pair.second.size())) {
      auto fileName = nextDir + "/" + to_string(fileNums++);
      SSTable::toSSTable(t, fileName, maxTimeStamp);
      cache.emplace(fileName, new SSTableCache(t, maxTimeStamp, fileName));
      valueSize = 0;
      length = 0;
      t.clear();
    }

    length++;
    valueSize += pair.second.size();
    t.push_back(pair);
  }

  // delete cache
  for (int i = 0; i < k; i++) {
    // delete cache
    assert(cache.count(compactionFiles[i]));
    auto c = cache.at(compactionFiles[i]);
    delete c;
    cache.erase(compactionFiles[i]);
  }

  // convert the remaining key value pairs to a SSTable
  if (!t.empty()) {
    auto fileName = nextDir + "/" + to_string(fileNums++);
    SSTable::toSSTable(t, fileName, maxTimeStamp);
    cache.emplace(fileName, new SSTableCache(t, maxTimeStamp, fileName));
  }

  // compact next level
  compaction(level + 1);
}

int KVStore::pow2(int n) {
  int ret = 1;
  for (int i = 0; i < n; i++) ret *= 2;
  return ret;
}

KVStore::~KVStore() {
  if (memTable.getLength() > 0) {
    if (!utils::dirExists(storagePath + "/level-0")) utils::mkdir((storagePath + "/level-0").c_str());
    auto fileName = storagePath + "/level-0/" + to_string(fileNums++);
    SSTable::toSSTable(memTable, fileName, timeStamp);
    cache.emplace(fileName, new SSTableCache(memTable, timeStamp, fileName));
    compaction(0);
  }

  for (auto &c:cache) delete c.second;
}

int KVStore::getLevel(const string &path) {
  auto s = path.substr(path.find("level-") + 6);
  return stoi(s);
}

