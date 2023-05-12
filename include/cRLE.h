#include <algorithm>
#include <array>
#include <iostream>
#include <iterator>
#include <limits>
#include <string_view>
#include <tuple>

using namespace std;

// CURRENTLY DOES NOT WORK WHEN STRING HAS NUMBERS... FIX.
// But like really doe, for genome sequences there are no numbers...

string encode(string inputstring) {
  auto first = inputstring.begin();
  auto last = inputstring.end();
  string out;
  char current_char = inputstring[0];
  int count = 1;
  for (int i = 1; i < inputstring.length(); i++) {
    // If not last, look for changes
    if (inputstring[i] != current_char) {
      if (count > 1) out += to_string(count);
      out += current_char;
      current_char = inputstring[i];
      count = 1;
    } else {
      count++;
    }
    // If its the last character we gotta figure out if we just swapped.
    if (i == inputstring.length() - 1) {
      if (count > 1) out += to_string(count);
      out += current_char;
      current_char = inputstring[i];
    }
  }

  return out;
};

string decode(string inputstring) {
  string out;
  string current_num = "";
  for (int i = 0; i < inputstring.length(); i++) {
    if (isdigit(inputstring[i])) {
      current_num += inputstring[i];
    } else {
      // no longer a digit, expend whatever this character is currentnum times.
      if (current_num != "")
        out += string(stoi(current_num), inputstring[i]);
      else
        out += inputstring[i];
      current_num = "";
    }
  }
  return out;
}
