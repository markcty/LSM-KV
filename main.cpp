#include <chrono>
#include <cstdlib>
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

    uniform_int_distribution<uint64_t> dist(1, 1024 * 1024 + 1000 * 1024);

    vector<int> time(100);
    vector<double> avgTime(100);
    vector<double> throughput(100);
    vector<int> count(100);

    auto totalSize = 0;
    unsigned long long totalTime = 0;
    for (int i = 0; i < max; i++) {
      if (i % 100 == 0) {
        cout << i << "...";
        cout.flush();
      }
      uint64_t key = dist(mt);
      totalSize += key;

      auto start = high_resolution_clock::now();
      store.put(key, string(key, 's'));
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
    cout << "0 ~ 0.25MB   : " << avgTime[0] << "ms "
         << "throughput: " << throughput[0] << "/s" << endl;
    cout << "0.25MB ~ 1MB : " << avgTime[1] << "ms "
         << "throughput: " << throughput[1] << "/s" << endl;
    cout << "0.5MB ~ 1MB  : " << avgTime[2] << "ms "
         << "throughput: " << throughput[2] << "/s" << endl;
    cout << "1MB ~ 1.5MB  : " << avgTime[3] << "ms "
         << "throughput: " << throughput[3] << "/s" << endl;
    cout << "1.5MB ~ 2MB  : " << avgTime[4] << "ms "
         << "throughput: " << throughput[4] << "/s" << endl;
    cout << "Average Delay: " << totalTime / (double)max << "ms "
         << "Average Throughput: " << 1 / (totalTime / (double)max) * 1000
         << "/s" << endl;
    cout << endl;

    cout << "Total Size Insert: " << totalSize / 1024 / 1024 << "MB" << endl;
    cout << "Total Time: " << totalTime / 1000 << "s" << endl;
  }
};

int main() {
  PutDelayTest putDelayTest("../data", 1000, false);
  putDelayTest.startTest();
}