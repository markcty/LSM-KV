#include <iostream>
#include <vector>
#include <fstream>
#include "utils.h"
#include <string>

int v;

using namespace std;
using SSTableDic = vector<pair<uint64_t, string>>;

void read64(ifstream &in, uint64_t &n) {
  in.read((char *) &n, 8);
}

void read32(ifstream &in, int &n) {
  in.read((char *) &n, 4);
}

void peek(string fileName) {
  ifstream in(fileName, ios_base::in | ios_base::binary);
  if (in.fail()) throw runtime_error("readDic: Open file " + fileName + " failed!");
  in.seekg(0, in.end);
  auto size = in.tellg();

  in.seekg(0, in.beg);
  uint64_t timeStamp, length, minKey, maxKey;
  read64(in, timeStamp);
  read64(in, length);
  read64(in, minKey);
  read64(in, maxKey);

  in.seekg(32 + 10240, in.beg);

  // read keys and offsets
  vector<uint64_t> keys;
  vector<int> offsets;
  for (int i = 0; i < length; i++) {
    uint64_t key;
    int offset;
    read64(in, key);
    read32(in, offset);
    keys.push_back(key);
    offsets.push_back(offset);
  }
  offsets.push_back(int(size - in.tellg()));

  SSTableDic dic;
  // read values
  for (int i = 0; i < length; i++) {
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

  cout << fileName << endl;
  cout << "***Header***" << endl;
  cout << timeStamp << endl
       << length << endl
       << minKey << endl
       << maxKey << endl
       << endl;

  if (v) {
    for (auto pair:dic) {
      cout << pair.first << ": " << pair.second.size() << endl;
    }
  }

  in.close();
}

int main() {

  int level;
  cin >> level;
  cin >> v;

  vector<string> files;
  string path = "./data/level-" + to_string(level);
  utils::scanDir(path, files);
  for (auto file:files) peek(path + "/" + file);
}
