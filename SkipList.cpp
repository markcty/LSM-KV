//
// Created by markcty on 2021/4/14.
//

#include "SkipList.h"

SkipList::Node::Node(uint64_t _key, string _value, Node *_right, Node *_down)
    : key(_key), value(std::move(_value)), right(_right), down(_down) {}

SkipList::Node::Node() : key(0), value(), right(nullptr), down(nullptr) {}

SkipList::SkipList() : head(new Node), length(0), size(0), rd(), mt(rd()) {}

void SkipList::put(uint64_t key, const string &value) {
  // if key exists, update the value of all nodes
  static auto update = [](Node *p, const string &value) {
    while (p) {
      p->value = value;
      p = p->down;
    }
  };

  // find the place to insert
  stack<Node *> path;
  Node *p = head;
  while (p) {
    while (p->right && p->right->key < key) p = p->right;
    // if key exists
    if (p->right && p->right->key == key) {
      update(p->right, value);
      return;
    }
    path.push(p);
    p = p->down;
  }

  // insert and grow up
  bool grow = true;
  Node *down = nullptr;
  while (grow && !path.empty()) {
    p = path.top();
    path.pop();
    p->right = new Node(key, value, p->right, down);
    down = p->right;
    grow = shouldGrowUp();
  }

  // height + 1
  if (grow) {
    p = head;
    head = new Node;
    head->right = new Node(key, value, nullptr, down);
    head->down = p;
  }

  length++;
  size += 8 + value.size();
}

const string *SkipList::get(uint64_t key) const {
  Node *p = head;
  while (p) {
    while (p->right && p->right->key < key) p = p->right;
    if (p->right && p->right->key == key) return &p->right->value;
    p = p->down;
  }
  return nullptr;
}

void SkipList::remove(uint64_t key) {
  // search the key
  stack<Node *> path;
  Node *p = head;
  while (p) {
    while (p->right && p->right->key < key) p = p->right;
    path.push(p);
    p = p->down;
  }

  // if the key exists?
  p = path.top()->right;
  if (p && p->key == key) {
    length--;
    size -= 8 + p->value.length();
  } else return;

  // remove the tower
  while (!path.empty()) {
    p = path.top();
    path.pop();
    Node *t = p->right;
    if (!t || t->key != key) return;
    p->right = t->right;
    delete t;
  }
}

bool SkipList::shouldGrowUp() {
  return mt() & 1;
}

void SkipList::reset() {
  while (head) {
    Node *t, *p = head;
    head = head->down;
    while (p) {
      t = p->right;
      delete p;
      p = t;
    }
  }
  head = new Node();
}

SkipList::~SkipList() {
  reset();
  delete head;
  length = 0;
  size = 0;
}

unsigned long SkipList::getLength() const { return length; }

unsigned long SkipList::getSize() const { return size; }

SkipList::Iterator SkipList::constBegin() const {
  Node *p = head;
  while (p->down) p = p->down;
  return SkipList::Iterator(p->right);
}

SkipList::Iterator SkipList::constEnd() { return SkipList::Iterator(nullptr); }

SkipList::Iterator::Iterator(SkipList::Node *_p) : p(_p) {}

const uint64_t &SkipList::Iterator::key() { return p->key; }

const string &SkipList::Iterator::value() { return p->value; }

SkipList::Iterator SkipList::Iterator::operator++() {
  p = p->right;
  return *this;
}

bool SkipList::Iterator::operator==(const SkipList::Iterator &other) const { return p == other.p; }

bool SkipList::Iterator::operator!=(const SkipList::Iterator &other) const { return p != other.p; }

#pragma clang diagnostic push
#pragma ide diagnostic ignored "readability-const-return-type"
const SkipList::Iterator SkipList::Iterator::operator++(int) {
  Iterator ret(p);
  p++;
  return ret;
}
#pragma clang diagnostic pop
