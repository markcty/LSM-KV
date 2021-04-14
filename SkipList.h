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
  struct Node {
    Node *right, *down;
    uint64_t key;
    string value;
    inline Node(uint64_t _key, string _value, Node *_right, Node *_down);
    inline Node();
  };
  Node *head;
  long size;
  mt19937 *randomEngine;
  bool shouldGrowUp();
 public:
  SkipList();

  ~SkipList();

  void put(uint64_t key, const string &value);

  const string *get(uint64_t key) const;

  void remove(uint64_t key);

  void reset();
};

#endif //LSM_KV__SKIPLIST_H_
