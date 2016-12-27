// Copyright: Peter Sanders. All rights reserved.
// Date: 2016-12-08
//
// This class defines a bitstring. It is similar to std::vector<bool>
// and std::bitset<>. From std::vector<bool>, it takes the dynamic size.
// From std::bitset<>, it takes the semantics and implied meaning.
//
// Additional functionality includes the ability to concatenate two bitstrings
// and access to the base memory pointer, a la std::vector<>.data(), which
// is missing from std::vector<bool> by design and must be missing from
// std::bitset due to the fixedness of the size.

#include <cstring>
#include <cstdint>

#include <vector>

#ifndef HUFFMAN_BASE_BITSTRING_
#define HUFFMAN_BASE_BITSTRING_

namespace hxtk {
class BitString {
 public:
  BitString() {}

  bool& at(int x) {
    this->
  }
  
  unit8_t* data() {
    return bytes_.data();
  }
 private:
  vector<uint8_t> bytes_;

};  // class bitstring
}  // namespace hxtk

#endif  // HUFFMAN_BASE_BITSTRING_
