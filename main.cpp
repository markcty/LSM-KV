#include <chrono>
#include <cstdlib>
#include <iostream>
#include <random>
#include <vector>

#include "kvstore.h"
#include "utils.h"

using namespace std;
using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;
using std::chrono::nanoseconds;

class PutDelayTest {
 private:
  KVStore store;
  int max;
  random_device rd;
  mt19937 mt;
  bool verbose;
  string path;

 public:
  explicit PutDelayTest(const string &path, int max_, bool verbose_)
      : rd(), mt(rd()), store(path), max(max_), verbose(verbose_), path(path) {}
  void startTest() {
    if (utils::dirExists(path)) {
      string cmd = "rm -rf " + path;
      system(cmd.c_str());
    }

    cout << ">>>>> Put Delay Test <<<<<" << endl;

    uniform_int_distribution<uint64_t> dist(1, 2 * 1024 * 1024 - 1024);

    unsigned long long totalTime = 0;
    for (int i = 0; i < max; i++) {
      uint64_t key = dist(mt);
      auto start = high_resolution_clock::now();
      store.put(key, string(key, 's'));
      auto end = high_resolution_clock::now();
      auto ms = duration_cast<milliseconds>(end - start).count();
      if (verbose) cout << ms << endl;
      totalTime += ms;
    }

    cout << "Average Delay: " << totalTime / (double)max << endl;
  }
};

int main() {
  PutDelayTest putDelayTest("../data", 1000, false);
  putDelayTest.startTest();
}