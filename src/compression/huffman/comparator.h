// Copyright: Peter Sanders. All rights reserved.
// Date: 2016-12-27
//
// This is a comparator class for comparing the values at two pointers
// It returns true if and only if the value at the first pointer is greater.

#include "compression/huffman/node.h"

namespace compression {
namespace huffman {
struct Comparator {
 public:
  bool operator()(Node* a, Node* b) {
    return (*a > *b);
  }
};  // struct Comparator
}  // namespace huffman
}  // namespace compression
