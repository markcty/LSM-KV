#include "BloomFilter.h"
#include "MurmurHash3.h"

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
  for (int i = 0; i < 10240 * 8; i += 8) {
    char temp = false;
    for (int j = 0; j <= 7; j++) temp |= bits[i + j] << j; // NOLINT(cppcoreguidelines-narrowing-conversions)
    out.write(&temp, sizeof(temp));
  }
}

BloomFilter::BloomFilter(const vector<pair<uint64_t, const string *>> &dic) {
  for (auto const &pair:dic) {
    unsigned int hash[4] = {0};
    MurmurHash3_x64_128(&pair.first, sizeof(pair.first), 1, &hash);
    bits[hash[0] % (10240 * 8)] = true;
    bits[hash[1] % (10240 * 8)] = true;
    bits[hash[2] % (10240 * 8)] = true;
    bits[hash[3] % (10240 * 8)] = true;
  }
}

