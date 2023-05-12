#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <filesystem>
#include <cmath>

#include "Progress.h"
#include "bwt.h"
#include "cRLE.h"
#include "omp.h"

using std::cout;
using std::endl;
namespace fs = std::filesystem;
#define DEBUG 0

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

std::string makePrintable(const std::string &s) {
  auto ls = s;
  for (auto &c : ls) {
    if (c == STX) {
      c = '^';
    } else if (c == ETX) {
      c = '|';
    }
  }
  return ls;
}

string readFile(string fn) {
  ifstream t(fn);
  stringstream buffer;
  buffer << t.rdbuf();
  return buffer.str();
}

void writeFile(string fn, string payload) {
  ofstream t(fn);
  t << payload;
  t.close();
}

string CompressBlock(string block, bool use_high_memory = false) {
  string encoded = "";
  try {
    string transformed = "";
    if (use_high_memory) {
      transformed = intonlybwt(block);
    } else {
      transformed = inplace_bwt(block);
    }
    writeFile("OUT", transformed);
    encoded = encode(transformed);
  } catch (std::runtime_error const &) {
    cout << "PROBLEM" << endl;
  }
  return encoded;
}

string DecompressBlock(string block) {
  string inverted = "";
  try {
    string decoded = decode(block);
    inverted = ibwt(decoded);
  } catch (std::runtime_error const &) {
    cout << "PROBLEM" << endl;
  }
  return inverted;
}

void WriteBlocks(string outfile, vector<string> blocks) {
  ofstream t(outfile);
  if (DEBUG) cout << "Blocks to write: " << blocks.size() << endl;
  t << blocks.size() << endl;
  for (int i = 0; i < blocks.size(); i++) {
    if (DEBUG) {
      cout << "Block [" << i << "] "
           << "Size: "
           << "[" << blocks.at(i).length() << "]" << endl;
    }
    t << blocks.at(i).length() << " ";
  }
  t << endl;
  for (int i = 0; i < blocks.size(); i++) {
    t << blocks.at(i);
  }
  t.close();
}

vector<string> ReadBlocks(string infile) {
  int blockcount, tmp;
  string input = readFile(infile);
  stringstream buf(input);
  buf >> blockcount;
  vector<int> counts;
  for (int i = 0; i < blockcount; i++) {
    buf >> tmp;
    counts.push_back(tmp);
  }
  // dump the rest of the stringstream buffer after space and endl.
  string data(buf.str().substr((int)buf.tellg() + 2));
  if (DEBUG) cout << "DATA: " << data << endl;
  vector<string> blocks;
  int cursor = 0;
  for (int i = 0; i < blockcount; i++) {
    blocks.push_back(data.substr(cursor, counts[i]));
    cursor += counts[i];
  }
  return blocks;
}

void Analyze(string originalinput, string inverseread, string original_fn, string compressed_fn) {
  cout << "\n\n------------- ANALYSIS -------------" << endl;
  cout.setf(std::cout.boolalpha);
  cout << "Round trip works? " << (originalinput == inverseread) << '\n';
  auto original_size = fs::file_size(original_fn);
  auto compressed_size = fs::file_size(compressed_fn);
  cout << "Original size: " << HumanReadable{original_size} << endl;
  cout << "Compressed size: " << HumanReadable{compressed_size} << endl;
  cout << "Bytes saved: " << (original_size - compressed_size) << endl;
  printf("Percentage of original file: %.2f%%\n", (double)(compressed_size / (double)original_size) * 100);
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    cout << "Wrong number of args please run as follows.\n./TextCompress <num_threads> <inputfile> <memory_mode>\n"
         << endl;
    exit(1);
  }
  bool highmem_mode = false;
  if (argc == 4) {
    if (stoi(argv[3]) == 1) {
      highmem_mode = true;
      cout << "![WARN]! Using high memory mode." << endl;
    }
  }
  ////////////////////////////////// Input
  cout << " ------ Reading Input ------ " << endl;
  string input = readFile(argv[2]);
  string compressed_fn = argv[2];
  compressed_fn += ".compress";
  CherryChrono::StopWatch pass(0);
  CherryChrono::StopWatch Total(0);
  Total.Start();
  pass.Start(" ------ Forward Pass Starting ------ ");
  int thread_count = stoi(argv[1]);
  int subrange = input.length() / thread_count;
  int residuals = input.length() % thread_count;
  // Work for each thread
  // number of blocks
  vector<string> blocks(thread_count);

#pragma omp parallel for num_threads(thread_count) shared(blocks)
  for (int i = 0; i < thread_count; i++) {
    if (i == thread_count - 1) {
      if (DEBUG) cout << input.substr(i * subrange, input.length()) << endl;
      blocks.at(i) = CompressBlock(input.substr(i * subrange, input.length()), highmem_mode);
    } else {
      if (DEBUG) cout << input.substr(i * subrange, subrange) << endl;
      blocks.at(i) = CompressBlock(input.substr(i * subrange, subrange), highmem_mode);
    }
  }
  WriteBlocks(compressed_fn, blocks);
  pass.End(" ------ Forward Pass Complete ------ ");
  pass.Start(" ------ Backward Pass Starting ------ ");
  vector<string> read_blocks = ReadBlocks(compressed_fn);
  for (auto &s : read_blocks) {
    if (DEBUG) cout << s << endl;
  }
  vector<string> decoded_blocks(read_blocks.size());
#pragma omp parallel for num_threads(thread_count) shared(read_blocks, decoded_blocks)
  for (int i = 0; i < read_blocks.size(); i++) {
    decoded_blocks.at(i) = DecompressBlock(read_blocks[i]);
  }
  string finished_string = "";
  for (auto &s : decoded_blocks) {
    finished_string += s;
  }
  if (DEBUG) cout << "Finished: " << finished_string << endl;
  pass.End(" ------ Backward Pass Complete ------ ");
  Analyze(input, finished_string, argv[2], compressed_fn);
  Total.End("Total Time (including middle IO):");
  return 0;
}