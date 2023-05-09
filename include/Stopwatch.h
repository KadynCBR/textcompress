#include <time.h>

#include <cstdio>
#include <iostream>
using namespace std;

namespace CherryChrono {

class StopWatch {
 public:
  StopWatch(int rank) { _rank = rank; }
  void Start(string outputstr = "") {
    if (_rank != 0) return;
    if (outputstr != "") cout << outputstr << endl;
    clock_gettime(CLOCK_MONOTONIC, &start);
    fstart = (double)start.tv_sec + (double)start.tv_nsec / 1000000000.0;
  }
  void End(string outputstr = "") {
    if (_rank != 0) return;
    clock_gettime(CLOCK_MONOTONIC, &now);
    fnow = (double)now.tv_sec + (double)now.tv_nsec / 1000000000.0;
    if (outputstr != "") cout << outputstr << endl;
    printf("%lf sec elapsed\n", (fnow - fstart));
  }

 private:
  double fnow;
  double fstart;
  int _rank;
  struct timespec now;
  struct timespec start;
};
}  // namespace CherryChrono
