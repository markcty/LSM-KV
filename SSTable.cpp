#include "SSTable.h"

#include <utility>
#include "MurmurHash3.h"

void SSTable::toSSTable(const SkipList &memTable, const string &fileName, uint64_t timeStamp) {
  ofstream out(fileName, ios::out | ios::binary | ios::trunc);
  if (out.fail()) throw runtime_error("toSSTable(SkipList): open file " + fileName + " failed!");

  auto length = memTable.getLength();
  auto min = memTable.getMinKey();
  auto max = memTable.getMaxKey();

  // Header
  write64(out, timeStamp);
  write64(out, length);
  write64(out, min);
  write64(out, max);

  // Bloom Filter
  BloomFilter bloomFilter(memTable);
  bloomFilter.write(out);

  // Key, Offset
  auto i = memTable.constBegin();
  uint32_t offset = 0;
  while (i.hasNext()) {
    i.next();
    write64(out, i.key());
    write32(out, offset);
    offset += i.value().size();
  }
  write64(out, 0);
  write32(out, offset);

  // Value
  i = memTable.constBegin();
  while (i.hasNext()) {
    i.next();
    out.write(i.value().c_str(), i.value().size());
  }

  out.close();
}

SSTableHeader SSTable::readHeader(const string &fileName) {
  ifstream in(fileName, ios_base::in | ios_base::binary);
  if (in.fail()) throw runtime_error("readHeader: Open file " + fileName + " failed!");
  uint64_t timeStamp, length, minKey, maxKey;
  read64(in, timeStamp);
  read64(in, length);
  read64(in, minKey);
  read64(in, maxKey);
  in.close();
  return SSTableHeader(timeStamp, length, minKey, maxKey);
}

void SSTable::readDic(const string &fileName, SSTableDic &dic) {
  ifstream in(fileName, ios_base::in | ios_base::binary);
  if (in.fail()) throw runtime_error("readDic: Open file " + fileName + " failed!");
  in.seekg(0, ifstream::end);
  auto size = in.tellg();

  uint64_t length;
  in.seekg(8, ifstream::beg); // skip timeStamp
  read64(in, length);
  in.seekg(10272, ifstream::beg); // skip header and bloomFilter

  // read keys and offsets
  vector<uint64_t> keys;
  vector<int> offsets;
  uint64_t key;
  int offset;
  for (int i = 0; i < length; i++) {
    read64(in, key);
    read32(in, offset);
    keys.push_back(key);
    offsets.push_back(offset);
  }
  read64(in, key);
  read32(in, offset);
  offsets.push_back(offset);

  // read values
  for (int i = 0; i < length; i++) {

    auto valueSize = offsets[i + 1] - offsets[i];
    char *buf = new char[valueSize + 1];
    in.read(buf, valueSize);
    buf[valueSize] = '\0';
    string value(buf);
    delete[] buf;
    dic.emplace_back(keys[i], value);
  }

  in.close();
}

void SSTable::toSSTable(const SSTableDic &dic, const string &fileName, uint64_t timeStamp) {
  ofstream out(fileName, ios::out | ios::binary | ios::trunc);
  if (out.fail()) throw runtime_error("toSSTable(dic): Open file " + fileName + " failed!");

  auto length = dic.size();
  auto min = dic[0].first;
  auto max = dic.back().first;

  // Header
  write64(out, timeStamp);
  write64(out, length);
  write64(out, min);
  write64(out, max);

  // BloomFilter
  BloomFilter bloomFilter(dic);
  bloomFilter.write(out);

  // Key, Offset
  uint32_t offset = 0;
  for (const auto &pair:dic) {
    write64(out, pair.first);
    write32(out, offset);
    offset += pair.second.size();
  }
  write64(out, 0);
  write32(out, offset);

  // Value
  for (const auto &pair:dic) {
    out.write(pair.second.c_str(), (long) pair.second.size());
  }

  out.close();
}

void SSTable::write64(ofstream &out, uint64_t n) {
  out.write((char *) &n, 8);
}

void SSTable::write32(ofstream &out, uint32_t n) {
  out.write((char *) &n, 4);
}

void SSTable::read64(ifstream &in, uint64_t &n) {
  in.read((char *) &n, 8);
}

void SSTable::read32(ifstream &in, int &n) {
  in.read((char *) &n, 4);
}

string SSTable::get(const string &fileName, uint64_t key) {
  ifstream in(fileName, ios_base::in | ios_base::binary);
  if (in.fail()) throw runtime_error("readDic: Open file " + fileName + " failed!");

  in.seekg(8, ifstream::beg); // skip timeStamp
  uint64_t length;
  read64(in, length);
  in.seekg(10272, ifstream::beg); // skip header and bloomFilter

  int offset = -1;
  int valueSize = -1;
  for (int i = 0; i < length; i++) {
    uint64_t _key;
    int _offset;
    read64(in, _key);
    read32(in, _offset);
    if (_key == key) {
      offset = _offset;
      read64(in, _key);
      read32(in, _offset);
      valueSize = _offset - offset;
      break;
    }
  }
  if (offset < 0) return "";
  in.seekg(10272 + (length + 1) * 12 + offset, ifstream::beg);
  char *buf = new char[valueSize + 1];
  in.read(buf, valueSize);
  buf[valueSize] = '\0';
  string value(buf);
  delete[] buf;

  in.close();
  return value;
}

SSTableHeader::SSTableHeader(uint64_t _timeStamp, uint64_t _size, uint64_t _minKey, uint64_t _maxKey) :
    timeStamp(_timeStamp), length(_size), minKey(_minKey), maxKey(_maxKey) {}

SSTableHeader::SSTableHeader() = default;

SSTableHeader::SSTableHeader(const SSTableHeader &other) = default;

BloomFilter::BloomFilter(const SkipList &memTable) {
  auto i = memTable.constBegin();
  while (i.hasNext()) {
    i.next();
    unsigned int hash[4] = {0};
    MurmurHash3_x64_128(&i.key(), sizeof(i.key()), 1, &hash);
    bits[hash[0] % (10240 * 8)] = true;
    bits[hash[1] % (10240 * 8)] = true;
    bits[hash[2] % (10240 * 8)] = true;
    bits[hash[3] % (10240 * 8)] = true;
  }
}

void BloomFilter::write(ofstream &out) {
  out.seekp(32, std::ofstream::beg);
  for (int i = 0; i < 10240 * 8; i += 8) {
    char temp = false;
    for (int j = 0; j <= 7; j++) temp |= bits[i + j] << j; // NOLINT(cppcoreguidelines-narrowing-conversions)
    out.write(&temp, sizeof(temp));
  }
}

BloomFilter::BloomFilter(const SSTableDic &dic) {
  for (const auto &pair:dic) {
    unsigned int hash[4] = {0};
    MurmurHash3_x64_128(&pair.first, sizeof(pair.first), 1, &hash);
    bits[hash[0] % (10240 * 8)] = true;
    bits[hash[1] % (10240 * 8)] = true;
    bits[hash[2] % (10240 * 8)] = true;
    bits[hash[3] % (10240 * 8)] = true;
  }
}

BloomFilter::BloomFilter(ifstream &in) {
  in.seekg(32, ifstream::beg);
  char buffer[10240];
  in.read(buffer, 10240);
  int p = 0;
  for (char c : buffer) {
    for (int i = 0; i < 8; i++)
      bits[p++] = (c >> i) & 1;
  }
}

bool BloomFilter::exists(uint64_t key) const {
  unsigned int hash[4] = {0};
  MurmurHash3_x64_128(&key, sizeof(key), 1, &hash);
  return bits[hash[0] % (10240 * 8)]
      && bits[hash[1] % (10240 * 8)]
      && bits[hash[2] % (10240 * 8)]
      && bits[hash[3] % (10240 * 8)];
}

BloomFilter::BloomFilter() = default;

SSTableCache::SSTableCache(const SkipList &memTable, uint64_t timeStamp, string _fileName)
    : fileName(std::move(_fileName)), bloomFilter(memTable) {
  header.length = memTable.getLength();
  header.minKey = memTable.getMinKey();
  header.maxKey = memTable.getMaxKey();
  header.timeStamp = timeStamp;

  auto i = memTable.constBegin();
  uint32_t offset = 0;
  while (i.hasNext()) {
    i.next();
    index.emplace_back(i.key(), offset);
    offset += i.value().size();
  }
  index.emplace_back(0, offset);
}

SSTableCache::SSTableCache(const SSTableDic &dic, uint64_t timeStamp, string _fileName)
    : fileName(std::move(_fileName)), bloomFilter(dic) {
  header.length = dic.size();
  header.minKey = dic.front().first;
  header.maxKey = dic.back().first;
  header.timeStamp = timeStamp;

  uint32_t offset = 0;
  for (const auto &pair:dic) {
    index.emplace_back(pair.first, offset);
    offset += pair.second.size();
  }
  index.emplace_back(0, offset);
}

string SSTableCache::get(uint64_t key) const {
  if (!(header.minKey <= key && key <= header.maxKey)) return "";
  if (!bloomFilter.exists(key)) return "";
  auto cmp = [](SSTableIndex left, uint64_t key) { return left.first < key; };
  auto low = lower_bound(index.begin(), index.end() - 1, key, cmp);
  if (low->first != key) return "";

  ifstream in(fileName, ios_base::in | ios_base::binary);
  if (in.fail()) throw runtime_error("readDic: Open file " + fileName + " failed!");

  auto offset = low->second;
  low++;
  int valueSize;
  valueSize = low->second - offset;

  in.seekg(10272 + (header.length + 1) * 12 + offset, ifstream::beg);

  char *buf = new char[valueSize + 1];
  in.read(buf, valueSize);
  buf[valueSize] = '\0';
  string value(buf);
  delete[] buf;

  in.close();
  return value;
}

SSTableHeader SSTableCache::getHeader() const {
  return header;
}

SSTableCache::SSTableCache(string _fileName) : fileName(std::move(_fileName)) {
  ifstream in(fileName, ios_base::in | ios_base::binary);
  if (in.fail()) throw runtime_error("readHeader: Open file " + fileName + " failed!");

  SSTable::read64(in, header.timeStamp);
  SSTable::read64(in, header.length);
  SSTable::read64(in, header.minKey);
  SSTable::read64(in, header.maxKey);

  bloomFilter = BloomFilter(in);

  in.seekg(32 + 10240, ifstream::beg);

  uint64_t key;
  int offset;
  for (int i = 0; i < header.length; i++) {
    SSTable::read64(in, key);
    SSTable::read32(in, offset);
    index.emplace_back(key, offset);
  }
  SSTable::read64(in, key);
  SSTable::read32(in, offset);
  index.emplace_back(key, offset);

  in.close();
}
