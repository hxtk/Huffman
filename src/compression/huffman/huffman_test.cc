// Copyright: Peter Sanders. All rights reserved.
// Date: 2017-01-26
//
// Unit test for Huffman class
// Assumes BitString class is sane

#include <cassert>

#include <iostream>
#include <fstream>
#include <string>

#include <base/bitstring.h>
#include <compression/huffman/huffman.h>

using std::cout;
using std::endl;
using std::string;

using compression::huffman::Huffman;
using base::BitString;

int main(int argc, char** argv) {
  string str = "";
  
  string tmp;
  while (cin >> tmp) {
    str += tmp + " ";
  }

  cout << "Using string:\n" << str << endl;

  /////////////////////////////////////////////////////////////////////////////
  // Encode and Decode, compare results
  Huffman huf;
  huf.BuildTree(str);
  huf.BuildMap();

  BitString bits;
  huf.Encode(str, &bits);

  cout << "Encoded to:\n" << bits << endl;

  cout << "==========TESTING DATA FIDELITY==========" << endl;
  cout << "Decoding . . ." << endl;

  int size = -1;
  char* decoded = nullptr;
  if (huf.Decode(bits, &decoded, &size)) {
    cout << "BitString sane" << endl;
  } else {
    cout << "Invalid BitString" << endl;
  }

  assert (decoded[size - 1] == 0);
  tmp = string(decoded);
  delete[] decoded;

  cout << "Decoded to:\n" << tmp << endl;
  cout << "Fidelity: " << (tmp == str) << endl;

  /////////////////////////////////////////////////////////////////////////////
  // Serialize and Unserialize, compare results
  // TODO: direct comparison of histograms
  void* buffer;
  int serial_size;
  huf.Serialize(&buffer, &size);

  Huffman huf2;
  huf2.Unserialize(buffer, size);

  cout << "==========TESTING HISTOGRAM FIDELITY==========" << endl;
  cout << "Decoding . . ." << endl;

  int size = -1;
  char* decoded = nullptr;
  if (huf2.Decode(bits, &decoded, &size)) {
    cout << "Histogram sane" << endl;
  } else {
    cout << "Bad histogram copy" << endl;
  }

  assert (decoded[size - 1] == 0);
  tmp = string(decoded);
  delete[] decoded;

  cout << "Decoded to:\n" << tmp << endl;
  cout << "Fidelity: " << (tmp == str) << endl;


  return 0;
}
