#include "SSTable.h"

void SSTable::toSSTable(const SkipList &memTable, const string &fileName, uint64_t timeStamp) {
  ofstream out(fileName, ios::out | ios::binary | ios::trunc);
  if (out.fail()) throw runtime_error("Open file " + fileName + " failed!");

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
    out.write(i.value().c_str(), (uint32_t) i.value().size());
  }

  out.close();
}

uint64_t SSTable::readTimeStamp(const string &fileName) {
  ifstream in(fileName, ios_base::in | ios_base::binary);
  uint64_t ret;
  in >> ret;
  in.close();
  return ret;
}

void SSTable::readDic(const string &fileName, vector<pair<uint64_t, string>> &dic) {
  ifstream in(fileName, ios_base::in | ios_base::binary);
  uint64_t size;
  in.seekg(8); // skip timeStamp
  in >> size;
  in.seekg(10272, ios_base::beg); // skip header and bloomFilter

  // read keys and offsets
  vector<uint64_t> keys;
  vector<uint32_t> offsets; // TODO: delete offsets
  for (int i = 0; i < size; i++) {
    uint64_t key;
    uint32_t offset;
    in >> key >> offset;
    keys.push_back(key);
    offsets.push_back(offset);
  }

  // read values
  for (int i = 0; i < size; i++) {
    string value;
    dic.emplace_back(keys[i], value);
  }
}

void SSTable::toSSTable(const vector<pair<uint64_t, const string *>> &dic, const string &fileName, uint64_t timeStamp) {

}

void SSTable::write64(ofstream &out, uint64_t n) {
  out.write((char *) &n, 8);
}

void SSTable::write32(ofstream &out, uint32_t n) {
  out.write((char *) &n, 4);
}
