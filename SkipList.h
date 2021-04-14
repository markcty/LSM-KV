//
// Created by markcty on 2021/4/14.
//

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

  // element numbers
  unsigned long length;

  // the number of bytes of keys and values occupy
  unsigned long size;

  // randomly generate a number to decide whether to grow up
  inline bool shouldGrowUp();

 public:
  SkipList();

  ~SkipList();

  void put(uint64_t key, const string &value);

  const string *get(uint64_t key) const;

  void remove(uint64_t key);

  void reset();

  inline unsigned long getLength() const;

  inline unsigned long getSize() const;

  class Iterator {
   public:
    explicit Iterator(Node *_p);
    const uint64_t &key();
    const string &value();
    Iterator operator++();
    const Iterator operator++(int);
    bool operator==(const Iterator &other) const;
    bool operator!=(const Iterator &other) const;
   private:
    Node *p;
  };

  Iterator constBegin() const;
  static Iterator constEnd();
};

#endif //LSM_KV__SKIPLIST_H_
