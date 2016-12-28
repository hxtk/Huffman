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

#include <cassert>
#include <cstring>
#include <cstdint>

#include <vector>
#include <iostream>
#include <ostream>

#ifndef HUFFMAN_BASE_BITSTRING_
#define HUFFMAN_BASE_BITSTRING_

namespace base {

static constexpr int kMaxByte = 256;  // Maximum unsigned 8-bit integer byte
static constexpr int kMaxBit = 128;   // Maximum value of a single bit in a byte
static constexpr int kByteBits = 8;   // Number of bits in a byte

class BitString {
 public:  
  BitString() {}

  // Assignment operator
  BitString& operator=(const BitString& rhs);

  // Store the provided value in the |index|th bit in the data byte array.
  // Here, the bit within the byte is selected such that the bits could
  // be read contiguously left-to-right.
  // Bounds checking is performed. New elements can NOT be added this way.
  bool Set(uint32_t index, bool value);

  // This function retreives the value at the given index by fetching the
  // |index % 8|th bit from the left of the |index / 8|th byte.
  // Bounds checking is performed.
  bool Get(uint32_t index) const;

  // Pack a new value onto the back of the array, incrementing the size.
  // Placement conventions are the same as those described in `Set()`
  void Append(bool value);

  // Pack the contents of bits onto the end of this array. This cannot be
  // achieved by copying memory unless the number of items already in the
  // array |= 0 (mod 8)|.
  void Append(const BitString& bits);

  // Removes the back item and reduces the size of the bitstring by one
  void PopBack();

  // Returns a byte array which contains a four byte header representing the
  // number of bits in the bitstring followed by |size_ / 8| bytes containing
  // the stored bits. The size header is necessary because the last byte may
  // contain between one (1) and eight (8) well-defined bits.
  // NOTE: the calling context is responsible for deleting this pointer
  void Serialize(uint8_t** buffer, int* size) const;

  // Given a string of the format described above, decode it to a bitstring.
  bool Unserialize(const char* input, int size);

  // Returns the number of bools packed in the container.
  uint32_t size() const {
    return size_;
  }
  void clear() {
    bytes_.resize(0);
    size_ = 0;
  }
  bool empty() {
    return (size_ == 0);
  }

  friend std::ostream& operator<<(std::ostream& lhs, const BitString& rhs) {
    for (int i = 0; i < rhs.size(); ++i) {
      lhs << rhs.Get(i);
      if (i % 4 == 3)
        lhs << " ";
    }
    lhs << std::flush;
    return lhs;
  }
 private:  
  std::vector<uint8_t> bytes_ = {};
  uint32_t size_ = 0;

};  // class bitstring
}  // namespace hxtk

#endif  // HUFFMAN_BASE_BITSTRING_
