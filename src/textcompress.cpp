#include "textcompress.h"

void TextCompress::ForwardPass(string input_fn, int num_threads) {
  file_in = input_fn;
  file_out = file_in + ".compress";
  string input = readFile(file_in);
  // This while a good idea, might be harmful in terms of seeing true speedup.
  block_count = ceil(input.length() / MAX_BLOCK_SIZE);
  subrange = MAX_BLOCK_SIZE;
  thread_count = num_threads;
  if (thread_count > block_count) {
    block_count = thread_count;
    subrange = input.length() / thread_count;
  }

  if (DEBUG) {
    cout << "Total size: " << input.length() << endl;
    cout << "# of blocks: " << block_count << endl;
  }
  // Work for each thread
  // number of blocks
  vector<string> blocks(block_count);
#pragma omp parallel for num_threads(thread_count) shared(blocks)
  for (int i = 0; i < block_count; i++) {
    if (i == block_count - 1) {
      if (DEBUG) cout << input.substr(i * subrange) << endl;
      blocks.at(i) = CompressBlock(input.substr(i * subrange));
    } else {
      if (DEBUG) cout << input.substr(i * subrange, subrange) << endl;
      blocks.at(i) = CompressBlock(input.substr(i * subrange, subrange));
    }
  }
  WriteBlocks(file_out, blocks);
  if (DEBUG) cerr << "Compressed file: " << file_out << endl;
}

void TextCompress::BackwardPass(string compressed_fn, int num_threads) {
  vector<string> read_blocks = ReadBlocks(compressed_fn);
  if (DEBUG) {
    for (auto &s : read_blocks) {
      cout << s << endl;
    }
  }
  vector<string> decoded_blocks(read_blocks.size());
#pragma omp parallel for num_threads(num_threads) shared(read_blocks, decoded_blocks)
  for (int i = 0; i < read_blocks.size(); i++) {
    decoded_blocks.at(i) = DecompressBlock(read_blocks[i]);
  }
  string finished_string = "";
  for (auto &s : decoded_blocks) {
    finished_string += s;
  }
  writeFile(compressed_fn.substr(0, compressed_fn.size() - 8) + "uncompress", finished_string);
  if (DEBUG) cout << "Finished: " << finished_string << endl;
  if (DEBUG) cerr << "Uncompressed file: " << compressed_fn.substr(0, compressed_fn.size() - 8) + "uncompress" << endl;
}

string TextCompress::CompressBlock(string block) {
  string encoded = "";
  string transformed = "";
  try {
    transformed = inplace_bwt(block);
    encoded = RLEencode(transformed);
  } catch (std::runtime_error const &) {
    cout << "PROBLEM" << endl;
  }
  return encoded;
}

string TextCompress::DecompressBlock(string block) {
  string inverted = "";
  try {
    string decoded = RLEdecode(block);
    inverted = ibwt(decoded);
  } catch (std::runtime_error const &) {
    cout << "PROBLEM" << endl;
  }
  return inverted;
}

vector<string> TextCompress::ReadBlocks(string infile) {
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

void TextCompress::WriteBlocks(string outfile, vector<string> blocks) {
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

string TextCompress::readFile(string fn) {
  ifstream t(fn);
  stringstream buffer;
  buffer << t.rdbuf();
  return buffer.str();
}

void TextCompress::writeFile(string fn, string payload) {
  ofstream t(fn);
  t << payload;
  t.close();
}

void TextCompress::Analyze(string original_fn, string compressed_fn) {
  string originalinput = readFile(original_fn);
  string inverseread = readFile(original_fn + ".uncompress");
  cout << "\n\n------------- ANALYSIS -------------" << endl;
  cout.setf(std::cout.boolalpha);
  cout << "Round trip works? " << (originalinput == inverseread) << '\n';
  // Signed, if the compressed file is bigger, this analysis is faulty.
  auto original_size = fs::file_size(original_fn);
  auto compressed_size = fs::file_size(compressed_fn);
  cout << "Original size: " << HumanReadable{original_size} << endl;
  cout << "Compressed size: " << HumanReadable{compressed_size} << endl;
  cout << "Bytes saved: " << (original_size - compressed_size) << endl;
  printf("Percentage of original file: %.2f%%\n", (double)(compressed_size / (double)original_size) * 100);
}

string TextCompress::inplace_bwt(string inp) {
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

void TextCompress::SetFirstLastPositions(vector<pair<CharacterPosition, CharacterPosition>> &firstlastPairs) {
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

unordered_map<char, vector<CharacterPosition>> TextCompress::makeLookupTable(
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

string TextCompress::ibwt(const string &r) {
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

// CURRENTLY DOES NOT WORK WHEN STRING HAS NUMBERS... FIX.
// But like really doe, for genome sequences there are no numbers...
string TextCompress::RLEencode(string inputstring) {
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
}

string TextCompress::RLEdecode(string inputstring) {
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

string TextCompress::makePrintable(const string &s) {
  auto ls = s;
  for (auto &c : ls) {
    if (c == STX) {
      c = '^';
    }
  }
  return ls;
}
