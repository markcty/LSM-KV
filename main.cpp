#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

#include "kvstore.h"
#include "utils.h"

using namespace std;
using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::microseconds;
using std::chrono::milliseconds;
using std::chrono::nanoseconds;
using std::chrono::seconds;

KVStore store("../data");

int main() {
  string path = "../data";

  auto start = high_resolution_clock::now();
  cout << ">>>>> Compaction Test <<<<<" << endl;

  int remain = 2 * 1024 * 1024 - 32 - 10240 - 1000 * 12;

  auto fre = {1, 5, 10, 25, 50, 100, 200, 400, 800, 1000};

  for (auto f : fre) {
    if (utils::dirExists(path)) {
      string cmd = "rm -rf " + path;
      system(cmd.c_str());
    }
    KVStore store(path);
    cout << f << " puts per compaction, ";
    string value(remain / f, 's');

    unsigned long long µs = 0;
    for (int i = 0; i < f * 100; i++) {
      auto s = high_resolution_clock::now();
      store.put(i, value);
      auto e = high_resolution_clock::now();
      µs += duration_cast<nanoseconds>(e - s).count();
    }
    cout << "Delay: " << µs / 1000 / 1000 / (double)(f * 100) << "ms" << endl;
  }

  if (utils::dirExists(path)) {
    string cmd = "rm -rf " + path;
    system(cmd.c_str());
  }

  cout << "no compaction, ";
  KVStore store(path);
  unsigned long long µs = 0;
  for (int i = 0; i < 2000000; i++) {
    auto s = high_resolution_clock::now();
    store.put(i, "s");
    auto e = high_resolution_clock::now();
    µs += duration_cast<nanoseconds>(e - s).count();
  }
  cout << "Delay: " << µs / 1000 / 1000 / (double)(2000000) << "ms" << endl;

  auto end = high_resolution_clock::now();
  auto totalTime = duration_cast<seconds>(end - start).count();

  cout << endl;
  cout << ">>>>> Summary <<<<<" << endl;
  cout << "It takes " << totalTime << "s to complete the test." << endl;
}