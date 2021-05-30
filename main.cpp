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
using std::chrono::seconds;

KVStore store("../data");

class PutDelayTest {
 private:
  int max;
  random_device rd;
  mt19937 mt;
  bool verbose;

 public:
  explicit PutDelayTest(int max_, bool verbose_)
      : rd(), mt(rd()), max(max_), verbose(verbose_) {}
  vector<int> startTest() {
    cout << ">>>>> Put Test <<<<<" << endl;

    uniform_int_distribution<uint64_t> dist(1, 1024 * 1024 + 1000 * 1024);

    vector<int> time(100);
    vector<double> avgTime(100);
    vector<double> throughput(100);
    vector<int> count(100);
    vector<int> keys;

    auto totalSize = 0;
    unsigned long long totalTime = 0;
    for (int i = 0; i < max; i++) {
      if (i % 100 == 0) {
        cout << i << "...";
        cout.flush();
      }
      uint64_t key = dist(mt);

      keys.push_back(key);
      totalSize += key;
      string value(key, 's');

      auto start = high_resolution_clock::now();
      store.put(key, value);
      auto end = high_resolution_clock::now();

      auto ms = duration_cast<milliseconds>(end - start).count();
      if (verbose) cout << ms << endl;

      if (key < 256 * 1024) {
        count[0]++;
        time[0] += ms;
      } else if (key < 512 * 1024) {
        count[1]++;
        time[1] += ms;
      } else if (key < 1024 * 1024) {
        count[2]++;
        time[2] += ms;
      } else if (key < 1536 * 1024) {
        count[3]++;
        time[3] += ms;
      } else {
        count[4]++;
        time[4] += ms;
      }

      totalTime += ms;
    }
    cout << endl;

    for (int i = 0; i < 5; i++) {
      avgTime[i] = time[i] / (double)count[i];
      throughput[i] = 1 / avgTime[i] * 1000;
    }

    cout << "Average Delay For Different Size Data: " << endl;
    cout << fixed << setprecision(2);
    cout << "0 ~ 0.25MB     : " << avgTime[0] << "ms "
         << "throughput: " << throughput[0] << "/s" << endl;
    cout << "0.25MB ~ 0.5MB : " << avgTime[1] << "ms "
         << "throughput: " << throughput[1] << "/s" << endl;
    cout << "0.5MB ~ 1MB    : " << avgTime[2] << "ms "
         << "throughput: " << throughput[2] << "/s" << endl;
    cout << "1MB ~ 1.5MB    : " << avgTime[3] << "ms "
         << "throughput: " << throughput[3] << "/s" << endl;
    cout << "1.5MB ~ 2MB    : " << avgTime[4] << "ms "
         << "throughput: " << throughput[4] << "/s" << endl;

    cout << endl;
    cout << "Average Delay: " << totalTime / (double)max << "ms, "
         << "Average Throughput: " << 1 / (totalTime / (double)max) * 1000
         << "/s" << endl;
    cout << "Total Size Insert: " << totalSize / 1024 / 1024 << "MB" << endl;
    cout << endl;
    return keys;
  }
};

class GetDelayTest {
 public:
  static void startTest(vector<int> &keys) {
    random_shuffle(keys.begin(), keys.end());

    vector<int> time(100);
    vector<double> avgTime(100);
    vector<double> throughput(100);
    vector<int> count(100);

    unsigned long long totalTime = 0;

    for (auto key : keys) {
      auto start = high_resolution_clock::now();
      store.get(key);
      auto end = high_resolution_clock::now();
      auto ms = duration_cast<microseconds>(end - start).count();

      if (key < 256 * 1024) {
        count[0]++;
        time[0] += ms;
      } else if (key < 512 * 1024) {
        count[1]++;
        time[1] += ms;
      } else if (key < 1024 * 1024) {
        count[2]++;
        time[2] += ms;
      } else if (key < 1536 * 1024) {
        count[3]++;
        time[3] += ms;
      } else {
        count[4]++;
        time[4] += ms;
      }

      totalTime += ms;
    }

    for (int i = 0; i < 5; i++) {
      avgTime[i] = time[i] / (double)count[i];
      throughput[i] = 1 / avgTime[i] * 1000 * 1000;
    }

    cout << ">>>>> Get Test <<<<<" << endl;
    cout << "Average Delay And Throughput For Different Size Data: " << endl;
    cout << fixed << setprecision(2);
    cout << "0 ~ 0.25MB     : " << avgTime[0] << "µs "
         << "throughput: " << throughput[0] << "/s" << endl;
    cout << "0.25MB ~ 0.5MB : " << avgTime[1] << "µs "
         << "throughput: " << throughput[1] << "/s" << endl;
    cout << "0.5MB ~ 1MB    : " << avgTime[2] << "µs "
         << "throughput: " << throughput[2] << "/s" << endl;
    cout << "1MB ~ 1.5MB    : " << avgTime[3] << "µs "
         << "throughput: " << throughput[3] << "/s" << endl;
    cout << "1.5MB ~ 2MB    : " << avgTime[4] << "µs "
         << "throughput: " << throughput[4] << "/s" << endl;
    cout << endl;
    cout << "Average Delay: " << totalTime / (double)keys.size() << "µs, "
         << "Average Throughput: "
         << 1 / (totalTime / (double)keys.size()) * 1000 * 1000 << "/s" << endl;
    cout << endl;
  }
};

class DeleteDelayTest {
 public:
  static void startTest(vector<int> &keys) {
    random_shuffle(keys.begin(), keys.end());

    vector<int> time(100);
    vector<double> avgTime(100);
    vector<double> throughput(100);
    vector<int> count(100);

    unsigned long long totalTime = 0;

    for (auto key : keys) {
      auto start = high_resolution_clock::now();
      store.del(key);
      auto end = high_resolution_clock::now();
      auto ms = duration_cast<microseconds>(end - start).count();

      if (key < 256 * 1024) {
        count[0]++;
        time[0] += ms;
      } else if (key < 512 * 1024) {
        count[1]++;
        time[1] += ms;
      } else if (key < 1024 * 1024) {
        count[2]++;
        time[2] += ms;
      } else if (key < 1536 * 1024) {
        count[3]++;
        time[3] += ms;
      } else {
        count[4]++;
        time[4] += ms;
      }

      totalTime += ms;
    }

    for (int i = 0; i < 5; i++) {
      avgTime[i] = time[i] / (double)count[i];
      throughput[i] = 1 / avgTime[i] * 1000 * 1000;
    }

    cout << ">>>>> Delete Test <<<<<" << endl;
    cout << "Average Delay And Throughput For Different Size Data: " << endl;
    cout << fixed << setprecision(2);
    cout << "0 ~ 0.25MB     : " << avgTime[0] << "µs "
         << "throughput: " << throughput[0] << "/s" << endl;
    cout << "0.25MB ~ 0.5MB : " << avgTime[1] << "µs "
         << "throughput: " << throughput[1] << "/s" << endl;
    cout << "0.5MB ~ 1MB    : " << avgTime[2] << "µs "
         << "throughput: " << throughput[2] << "/s" << endl;
    cout << "1MB ~ 1.5MB    : " << avgTime[3] << "µs "
         << "throughput: " << throughput[3] << "/s" << endl;
    cout << "1.5MB ~ 2MB    : " << avgTime[4] << "µs "
         << "throughput: " << throughput[4] << "/s" << endl;
    cout << endl;
    cout << "Average Delay: " << totalTime / (double)keys.size() << "µs, "
         << "Average Throughput: "
         << 1 / (totalTime / (double)keys.size()) * 1000 * 1000 << "/s" << endl;
    cout << endl;
  }
};

int main() {
  string path = "../data";
  if (utils::dirExists(path)) {
    string cmd = "rm -rf " + path;
    system(cmd.c_str());
  }

  auto start = high_resolution_clock::now();
  PutDelayTest putDelayTest(1000, false);
  auto keys = putDelayTest.startTest();

  GetDelayTest::startTest(keys);
  DeleteDelayTest::startTest(keys);

  if (utils::dirExists(path)) {
    string cmd = "rm -rf " + path;
    system(cmd.c_str());
  }

  auto end = high_resolution_clock::now();
  auto totalTime = duration_cast<seconds>(end - start).count();

  cout << ">>>>> Summary <<<<<" << endl;
  cout << "It takes " << totalTime << "s to complete the test." << endl;
}