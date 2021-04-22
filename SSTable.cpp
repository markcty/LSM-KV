#include "SSTable.h"
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

  // Value
  i = memTable.constBegin();
  while (i.hasNext()) {
    i.next();
    out.write(i.value().c_str(), (long) i.value().size());
  }

  out.close();
}

SSTableHeader SSTable::readHeader(const string &fileName) {
  ifstream in(fileName, ios_base::in | ios_base::binary);
  if (in.fail()) throw runtime_error("readHeader: Open file " + fileName + " failed!");
  uint64_t timeStamp, size, minKey, maxKey;
  read64(in, timeStamp);
  read64(in, size);
  read64(in, minKey);
  read64(in, maxKey);
  in.close();
  return SSTableHeader(timeStamp, size, minKey, maxKey);
}

void SSTable::readDic(const string &fileName, SSTableDic &dic) {
  ifstream in(fileName, ios_base::in | ios_base::binary);
  if (in.fail()) throw runtime_error("readDic: Open file " + fileName + " failed!");
  in.seekg(0, ios_base::end);
  auto fileSize = in.tellg();

  uint64_t size;
  in.seekg(8, ios_base::beg); // skip timeStamp
  read64(in, size);
  in.seekg(10272, ios_base::beg); // skip header and bloomFilter

  // read keys and offsets
  vector<uint64_t> keys;
  vector<uint32_t> offsets; // TODO: delete offsets
  for (int i = 0; i < size; i++) {
    uint64_t key;
    uint32_t offset;
    read64(in, key);
    read32(in, offset);
    keys.push_back(key);
    offsets.push_back(offset);
  }
  offsets.push_back(fileSize - in.tellg());

  // read values
  for (int i = 0; i < size; i++) {
    string value;
    auto valueSize = offsets[i + 1] - offsets[i];
    value.reserve(offsets[i + 1] - offsets[i]);
    while (valueSize--) {
      char c;
      in.read(&c, 1);
      value.push_back(c);
    }
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

void SSTable::read32(ifstream &in, uint32_t &n) {
  in.read((char *) &n, 4);
}

SSTableHeader::SSTableHeader(uint64_t _timeStamp, uint64_t _size, uint64_t _minKey, uint64_t _maxKey) :
    timeStamp(_timeStamp), size(_size), minKey(_minKey), maxKey(_maxKey) {}

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
  out.seekp(32, ios_base::beg);
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
  in.seekg(32, ios_base::beg);
  char buffer[10240];
  in.read(buffer, 10240);
  int p = 0;
  for (char c : buffer) {
    for (int i = 0; i < 8; i++)
      bits[p++] = (c >> i) & 1;
  }
}
