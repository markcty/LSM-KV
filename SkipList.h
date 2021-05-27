#ifndef LSM_KV__SKIPLIST_H_
#define LSM_KV__SKIPLIST_H_

#include <string>
#include <stack>
#include <utility>
#include <random>

using namespace std;

class SkipList {
 private:
  random_device rd;
  mt19937 mt;

  struct Node {
    Node *right, *down;
    uint64_t key;
    string value;
    inline Node(uint64_t _key, string _value, Node *_right, Node *_down);
    inline Node();
  };
  Node *head;

  unsigned long length;

  unsigned long valueSize;

  inline bool shouldGrowUp();

 public:
  SkipList();

  ~SkipList();

  void put(uint64_t key, const string &value);

  string get(uint64_t key) const;

  bool remove(uint64_t key);

  void reset();

  unsigned long getLength() const;

  unsigned long getValueSize() const;

  uint64_t getMinKey() const;

  uint64_t getMaxKey() const;

  class ConstIterator {
   public:
    explicit ConstIterator(Node *_p);
    const uint64_t &key();
    const string &value();
    bool hasNext() const;
    void next();
   private:
    Node *p;
  };

  ConstIterator constBegin() const;
};

#endif //LSM_KV__SKIPLIST_H_
