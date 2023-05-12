#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <execution>
#include "Stopwatch.h"
using namespace std;

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

string inplace_bwt(string inp) {
  string rets = inp;
  rets += STX;
  int n = rets.length();
  int i, p, r, s;
  unsigned char c;
  for (s = n - 3; s >= 0; s--) {
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
    currentToken = LUT[currentToken.character][currentToken.position];
    if (currentToken.character != STX) out = currentToken.character + out;
  }
  return out;
}