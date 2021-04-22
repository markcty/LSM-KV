#include "SkipList.h"

SkipList::Node::Node(uint64_t _key, string _value, Node *_right, Node *_down)
    : right(_right), down(_down), key(_key), value(std::move(_value)) {}

SkipList::Node::Node() : right(nullptr), down(nullptr), key(0), value() {}

SkipList::SkipList() : rd(), mt(rd()), head(new Node), length(0), size(0) {}

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
  size += keySize + value.size();
}

string SkipList::get(uint64_t key) const {
  Node *p = head;
  while (p) {
    while (p->right && p->right->key < key) p = p->right;
    if (p->right && p->right->key == key) return p->right->value;
    p = p->down;
  }
  return "";
}

bool SkipList::remove(uint64_t key) {
  // search the key
  stack<Node *> path;
  Node *p = head;
  while (p) {
    while (p->right && p->right->key < key) p = p->right;
    path.push(p);
    p = p->down;
  }

  // Does the key exist?
  p = path.top()->right;
  if (!p || p->key != key) return false;

  length--;
  size -= keySize + p->value.length();
  // remove the tower
  while (!path.empty()) {
    p = path.top();
    path.pop();
    Node *t = p->right;
    if (!t || t->key != key) return true;
    p->right = t->right;
    delete t;
  }
  return true;
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
  length = 0;
  size = 0;
}

SkipList::~SkipList() {
  reset();
  delete head;
}

SkipList::ConstIterator SkipList::constBegin() const {
  Node *p = head;
  while (p->down) p = p->down;
  return SkipList::ConstIterator(p);
}

uint64_t SkipList::getMinKey() const {
  Node *p = head;
  while (p->down) p = p->down;
  if (p->right) return p->right->key;
  return 0;
}

uint64_t SkipList::getMaxKey() const {
  Node *p = head;
  while (p->down) p = p->down;
  while (p->right) p = p->right;
  return p->key;
}

unsigned long SkipList::getLength() const { return length; }

unsigned long SkipList::getSize() const { return size; }

SkipList::ConstIterator::ConstIterator(SkipList::Node *_p) : p(_p) {}

const uint64_t &SkipList::ConstIterator::key() { return p->key; }

const string &SkipList::ConstIterator::value() { return p->value; }

bool SkipList::ConstIterator::hasNext() const { return p->right; }

void SkipList::ConstIterator::next() { p = p->right; }
