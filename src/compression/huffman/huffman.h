// Copyright: Peter Sanders. All rights reserved.
// Date: 2016-11-30
//
// This class encodes and decodes strings using Huffman Coding.
// Several methods must be called in order, depending on the use.
//
// Encoding a string:
//      std::string str = ...
//      Huffman huf;
//      huf.BuildTree(str);
//      huf.BuildMap();
//      vector<bool> encoded = huf.Encode(str);
//
// Decoding a bitstring:
//      std::vector<bool> bits = ...
//      const char* histogram = ...
//      Huffman huf;
//      huf.Unserialize(histogram);
//      std::string decoded = huf.Decode(bits);
//
//      OR
//
//      std::string str = ...
//      Huffman huf;
//      huf.BuildTree(str);
//      /* ... */
//      std::vector<bool> bits = ...
//      std::string decoded = huf.Decode(bits)
//
// Saving a histogram:
//      Huffman huf;
//      /* ... */
//      std::ostream stream(...
//      char* buffer = huf.Serialize();
//      stream.write(buffer, huf.GetSerialBufferSize());
//      delete[] buffer;

#ifndef HUFFMAN_HUFFMAN_H_
#define HUFFMAN_HUFFMAN_H_

#include <cstdint>
#include <cstring>

#include <string>
#include <vector>
#include <queue>
#include <unordered_map>

#include "huffman/node.h"
#include "base/bitstring.h"

namespace huffman {
class Huffman {
 public:  
  Huffman() {}

  ~Huffman() {
    delete tree_;
  }

  // This accepts a string and builds the Huffman Coding Tree
  // for that string. This method OR |SetTree| must be called
  // before Encode or Decode.
  //
  // First, a histogram is constructed of the symbols found
  // in the string. This histogram is used to sort the symbols
  // in descending order by frequency.
  //
  // Next, execution is passed off to |BuildTree()|
  void BuildTree(const std::string& text);
  void BuildTree(const void* text, int size);

  // NOTE: This must be called AFTER |BuildTree| or |Unserialize|
  //
  // This function searches through the binary tree
  // to generate a map from letters to their associated
  // bitstrings.
  //
  // Returns |true| on success, |false| on failure.
  // Failure can indicate that the machine ran out of memory
  // or that the tree has not yet been initialized.
  bool BuildMap();

  // NOTE: This function must be called AFTER |BuildMap|
  //
  // This function accepts a string and encodes it using the Huffman Tree
  // A bitstring is then returned containing the encoded bytestring.
  void Encode(const std::string& text, base::BitString* bits) const;
  void Encode(const uint8_t* text, int size, base::BitString* bits) const;

  // NOTE: This function must be called AFTER |BuildTree| or |Unserialize|
  // NOTE: This function does NOT depend on |BuildMap|
  //
  // This function accepts a bitstring and converts it into a bytestring
  // using the Huffman Coding Tree.
  //
  // Traverse down the tree according to the bit.
  // A |1| bit indicates (right). A |0| bit indicates left.
  // If the current node is a leaf node, add it to the buffer
  // and reset current node to root.
  std::vector<uint8_t> Decode(const base::BitString& bits) const;

  // This function returns a pointer to a buffer
  // containing the canonical byte representation of the histogram.
  // This is all of the information one would need to reconstruct
  // the Huffman tree deterministically. As such it is used as
  // the canonical serialization of the object.
  //
  // The format begins with a header. This is an unsigned 8-bit int |n|
  // giving the number of non-zero entries in the histogram.
  // This will be at most 204. If there are more than 204 entries
  // in the histogram, there is a more efficient method.
  //
  // Subsequently, there will be |n| 5-byte blocks. The first byte is a `char`
  // indicating the character whose data is found subsequently.
  // The remaining bytes are a 32-bit integer representing the number of times
  // that byte occurs in the string.
  //
  // If there are more than 204, the header byte shall be |0| and it shall
  // be followed by exactly 256 32-bit integers, which will be the full data
  // of the histogram. Because [[205*(5 bytes) > 256*(4 bytes)]], this is
  // more space-efficient than storing only non-zero values for any histogram
  // with more than 204 unique entries.
  void Serialize(void** buffer, int* size) const;

  // This accepts the standard serialized string and initializes the object
  // such that it matches the one that was serialized.
  // This is accomplished by first initializing the histogram from the serial
  // string, and then calling |BuildTree()|
  bool Unserialize(const void* bytes, int size);

  // This returns the canonical string form of the Huffman Coding Tree
  std::string ToString() const;

  // Given a buffer containing a huffman archive
  // return a pointer to the beginning of the data segment.
  static const uint8_t* get_data_segment(const uint8_t* buffer) {
    buffer + Huffman::get_header_size(buffer);
  }

  static int get_header_size(const uint8_t* buffer) {
    if(buffer[0] == 0) {
      return 1 + base::kMaxByte*sizeof(int32_t);
    } else {
      return 1 + kEntryWidth*buffer[0];
    }
  }

 private:
  static constexpr int kBreakEvenHistogramSize = 204;
  static constexpr int kEntryWidth = sizeof(uint8_t) + sizeof(int32_t);

  // This is the meat of the |BuildTree| function described above.
  // Using a min heap, the two smallest elements are removed and put back
  // as a single branch node with value equaling the sum of its children.
  // This continues until there is only one node remaining.
  void BuildTree();
  
  // These are the recursive calls for the associated public functions
  // of the same name.
  std::string ToString(Node* fakeroot, int depth) const;
  bool BuildMap(Node* fakeroot, base::BitString* bits);

  Node* tree_ = nullptr;
  std::unordered_map<uint8_t, base::BitString> encode_map_ = {};
  std::vector<int32_t> histogram_ = {};
};  // class Huffman
}  // namespace huffman

#endif  // HUFFMAN_HUFFMAN_H_
