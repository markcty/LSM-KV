#include <iostream>
#include "SkipList.h"

using namespace std;

int main() {
  SkipList l;
  l.put(19, "haha");
  l.put(12, "haa");
  l.put(22, "ha");
  l.put(33, "4444");
  l.put(33, "h");
  l.put(19, "hhhh");
  l.remove(22);
  cout << *l.get(19) << endl << *l.get(33) << *l.get(33) << endl;
  return 0;
}
