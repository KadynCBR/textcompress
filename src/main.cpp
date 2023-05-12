#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "omp.h"
#include "textcompress.h"

using std::cout;
using std::endl;
namespace fs = std::filesystem;

int main(int argc, char *argv[]) {
  if (argc < 3) {
    cout << "Wrong number of args please run as follows.\n./TextCompress <num_threads> <inputfile>\n" << endl;
    exit(1);
  }
  string input_fn = argv[2];
  int num_threads = stoi(argv[1]);
  TextCompress tc;
  CherryChrono::StopWatch pass(0);
  CherryChrono::StopWatch Total(0);
  Total.Start();
  pass.Start(" ------ Forward Pass Starting ------ ");
  tc.ForwardPass(input_fn, num_threads);
  pass.End(" ------ Forward Pass Complete ------ ");
  pass.Start(" ------ Backward Pass Starting ------ ");
  string compressed_fn = input_fn + ".compress";
  tc.BackwardPass(compressed_fn, num_threads);
  pass.End(" ------ Backward Pass Complete ------ ");
  string uncompressed_fn = input_fn + ".uncompress";
  tc.Analyze(input_fn, compressed_fn);
  Total.End("Total Time (including middle IO):");
  return 0;
}