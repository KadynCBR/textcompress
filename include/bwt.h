#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Progress.h"
#include "Stopwatch.h"
using namespace std;

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

const int STX = 0x02;
const int ETX = 0x03;

void rotate(string &a) {
  char t = a[a.length() - 1];
  for (int i = a.length() - 1; i > 0; i--) {
    a[i] = a[i - 1];
  }
  a[0] = t;
}

string inplace_bwt(string inp) {
  string rets = inp;
  rets += STX;
  int n = rets.length();
  int i, p, r, s;
  unsigned char c;
  for (s = n - 3; s >= 0; s--) {
    // CherryProg::printProgress(double(n - s) / double(n));
    c = rets[s];

    r = s;
    for (i = s + 1; rets[i] != STX; i++)
      if (rets[i] <= c) r++;
    p = i;
    while (i < n)
      if (rets[i++] < c) r++;

    rets[p] = c;

    for (i = s; i < r; i++) {
      rets[i] = rets[i + 1];
    }
    rets[r] = STX;
  }
  return rets;
}

string bwt(const string &s) {
  for (char c : s) {
    if (c == STX || c == ETX) {
      throw runtime_error("Input can't contain STX or ETX");
    }
  }
  string ss;
  ss += STX;
  ss += s;
  // ss += ETX;
  // push all rotations to a vector
  vector<string> table;
  for (size_t i = 0; i < ss.length(); i++) {
    table.push_back(ss);
    rotate(ss);
  }
  // sort rotations
  sort(table.begin(), table.end());
  string out;
  for (auto &s : table) {
    out += s[s.length() - 1];  // Grab the last character of each rotation after sort
  }
  return out;
}

void SetFirstLastPositions(vector<pair<CharacterPosition, CharacterPosition>> &firstlastPairs) {
  unordered_map<char, int> chrcounts_first;
  unordered_map<char, int> chrcounts_last;
  for (auto &flpair : firstlastPairs) {
    char fchar = flpair.first.character;
    char lchar = flpair.second.character;
    // if the character doesn't exist in the map, make it zero else add 1
    if (chrcounts_first.find(fchar) == chrcounts_first.end()) {
      chrcounts_first.insert(make_pair(fchar, 0));
    } else {
      chrcounts_first[fchar]++;
    }
    flpair.first.position = chrcounts_first[fchar];

    if (chrcounts_last.find(lchar) == chrcounts_last.end()) {
      chrcounts_last.insert(make_pair(lchar, 0));
    } else {
      chrcounts_last[lchar]++;
    }
    flpair.second.position = chrcounts_last[lchar];
  }
}

unordered_map<char, vector<CharacterPosition>> makeLookupTable(
    vector<pair<CharacterPosition, CharacterPosition>> &firstlastPairs) {
  // LUT["b"][3] -> corresponds to the first's array's b(k=3)'s last pair.
  unordered_map<char, vector<CharacterPosition>> LUT;
  for (auto &flpair : firstlastPairs) {
    CharacterPosition fchar = flpair.first;
    CharacterPosition lchar = flpair.second;
    vector<CharacterPosition> currenttable;
    if (!(LUT.find(fchar.character) == LUT.end())) currenttable = LUT[fchar.character];

    if (fchar.position >= currenttable.size()) {
      currenttable.resize(fchar.position + 1);
    }
    currenttable[fchar.position] = flpair.second;
    LUT[fchar.character] = currenttable;
  }

  return LUT;
}

// We can make this faster using First Last Principle
string ibwt(const string &r) {
  string sortedR = r;
  sort(sortedR.begin(), sortedR.end());
  vector<pair<CharacterPosition, CharacterPosition>> firstlastPairs;
  for (int i = 0; i < r.length(); i++) {
    firstlastPairs.push_back(
        pair<CharacterPosition, CharacterPosition>(CharacterPosition(sortedR[i]), CharacterPosition(r[i])));
  }
  SetFirstLastPositions(firstlastPairs);
  unordered_map<char, vector<CharacterPosition>> LUT = makeLookupTable(firstlastPairs);
  // Begin F-L pair hop back
  CharacterPosition currentToken = CharacterPosition(STX);
  currentToken.position = 0;
  string out;
  // Account for not accepting the STX
  while (out.length() < r.length() - 1) {
    // CherryProg::printProgress(double(out.length()) / double(r.length() - 2));
    currentToken = LUT[currentToken.character][currentToken.position];
    if (currentToken.character != STX) out = currentToken.character + out;
  }
  return out;
}