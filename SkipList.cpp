//
// Created by markcty on 2021/4/14.
//

#include "SkipList.h"

SkipList::Node::Node(uint64_t _key, string _value, Node *_right, Node *_down)
    : key(_key), value(std::move(_value)), right(_right), down(_down) {
}

SkipList::Node::Node() : key(0), value(), right(nullptr), down(nullptr) {
}

SkipList::SkipList() : head(new Node), size(0) {
  random_device rd;
  randomEngine = new mt19937(rd());
}

void SkipList::put(uint64_t key, const string &value) {
  // if key exists, update the value of all nodes
  static auto update = [](Node *p, const string &value) {
    while (p) {
      p->value = value;
      p = p->down;
    }
  };

  // find place to insert
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
  stack<Node *> path;
  Node *p = head;
  while (p) {
    while (p->right && p->right->key < key) p = p->right;
    path.push(p);
    p = p->down;
  }
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
  return randomEngine->operator()() & 1;
}

void SkipList::reset() {
  while (head) {
    Node *t, *p;
    t = p = head;
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
}
