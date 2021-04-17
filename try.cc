#include <iostream>
#include "SkipList.h"
#include "SSTable.h"

using namespace std;

int main() {
  SkipList l;
  l.put(19, "19 ");
  l.put(12, "12 ");
  l.put(22, "22 ");
  l.put(33, "33 ");
  l.put(33, "33  ");
  l.put(19, "19  ");
  l.put(1000, "1000 ");
  l.remove(22);
  auto i = l.constBegin();
  while (i.hasNext()) {
    i.next();
    cout << i.key() << ":" << i.value() << endl;
  }
  SSTable::toSSTable(l, "./data/level-0/0.sst", 0);
  return 0;
}