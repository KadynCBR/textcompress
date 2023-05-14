#include <algorithm>
#include <array>
#include <cmath>
#include <execution>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Stopwatch.h"
#include "omp.h"

using namespace std;
namespace fs = std::filesystem;

const int STX = 0x02;

struct CharacterPosition {
  CharacterPosition() {
    character = '_';
    position = -1;
  }
  CharacterPosition(char c) {
    character = c;
    position = -1;
  }
  int position;
  char character;
};

struct HumanReadable {
  std::uintmax_t size{};

 private:
  friend std::ostream &operator<<(std::ostream &os, HumanReadable hr) {
    int i{};
    double mantissa = hr.size;
    for (; mantissa >= 1024.; mantissa /= 1024., ++i) {
    }
    mantissa = std::ceil(mantissa * 10.) / 10.;
    os << mantissa << "BKMGTPE"[i];
    return i == 0 ? os : os << "B (" << hr.size << ')';
  }
};

class TextCompress {
 public:
  void ForwardPass(string input_fn, int num_threads);
  void BackwardPass(string compressed_fn, int num_threads);
  string CompressBlock(string block);
  string DecompressBlock(string block);
  vector<string> ReadBlocks(string infile);
  void WriteBlocks(string outfile, vector<string> blocks);
  string readFile(string fn);
  void writeFile(string fn, string payload);
  void Analyze(string original_fn, string compressed_fn);

 private:
  string inplace_bwt(string inp);
  void SetFirstLastPositions(vector<pair<CharacterPosition, CharacterPosition>> &firstlastPairs);
  unordered_map<char, vector<CharacterPosition>> makeLookupTable(
      vector<pair<CharacterPosition, CharacterPosition>> &firstlastPairs);
  string ibwt(const string &r);
  string RLEencode(string inputstring);
  string RLEdecode(string inputstring);
  string makePrintable(const string &s);

  string file_in;
  string file_out;
  int subrange;
  int thread_count;
  int block_count;
  int MAX_BLOCK_SIZE = 500000;
  bool DEBUG = false;
};