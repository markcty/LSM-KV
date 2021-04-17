#include "SSTable.h"

void SSTable::toSSTable(const SkipList &memTable, const string &fileName, uint64_t timeStamp) {
  ios_base::sync_with_stdio(false);
  ofstream out(fileName, ios::out | ios::binary | ios::trunc);
  if (out.fail()) throw runtime_error("Open file " + fileName + " failed!");

  static auto write64 = [&out](uint64_t n) {
    out.write((char *) &n, 8);
  };

  static auto write32 = [&out](uint32_t n) {
    out.write((char *) &n, 4);
  };

  auto length = memTable.getLength();
  auto min = memTable.getMinKey();
  auto max = memTable.getMaxKey();

  // Header
  write64(timeStamp);
  write64(length);
  write64(min);
  write64(max);

  // Bloom Filter
  BloomFilter bloomFilter(memTable);
  bloomFilter.write(out);

  // Key, Offset
  auto i = memTable.constBegin();
  uint32_t offset = 0;
  while (i.hasNext()) {
    i.next();
    write64(i.key());
    write32(offset);
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
