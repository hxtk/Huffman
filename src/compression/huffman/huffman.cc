// Copyright: Peter Sanders. All rights reserved.
// Date: 2016-11-30

#include "compression/huffman/huffman.h"

#include <cassert>
#include <cstdint>
#include <cstring>  

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <queue>    
#include <unordered_map>

#include "compression/huffman/node.h"
#include "compression/huffman/comparator.h"

#include "base/bitstring.h"

using std::string;
using std::vector;
using std::priority_queue;
using std::unordered_map;

using base::BitString;

using std::cout;
using std::endl;

namespace compression {
namespace huffman {
void Huffman::BuildTree(const void* text, int size) {
  const uint8_t* values_ptr = reinterpret_cast<const uint8_t*>(text);

  histogram_ = vector<uint32_t>(base::kMaxByte, 0);
  for (int i = 0; i < size; ++i) {
    ++histogram_.at(values_ptr[i]);
  }
  this->BuildTree();
}

void Huffman::BuildTree() {
  // Create a node for each item in the histogram
  priority_queue<Node*, vector<Node*>, Comparator> nodes;
  for (int i = 0; i < base::kMaxByte; ++i) {
    nodes.push(Node::BuildLeaf(i, histogram_.at(i)));
  }

  // Reduce the forest to a single tree
  while (nodes.size() > 1) {
    Node* a = nodes.top();
    nodes.pop();
    Node* b = nodes.top();
    nodes.pop();

    nodes.push(Node::BuildBranch(a, b));
  }
  tree_ = nodes.top();
}

bool Huffman::BuildMap() {
  if (tree_ == nullptr) return false;

  BitString bits;
  BuildMap(tree_, &bits);

  return true;
}

bool Huffman::BuildMap(Node* fakeroot, BitString* bits) {
  if (fakeroot->is_leaf()) {
    encode_map_.emplace(fakeroot->get_symbol(), *bits);
    return true;
  }
  bool res = true;

  // Traverse into the left branch
  bits->Append(false);
  res &= BuildMap(fakeroot->get_left(), bits);
  bits->PopBack();

  // Traverse into the right branch
  bits->Append(true);
  res &= BuildMap(fakeroot->get_right(), bits);
  bits->PopBack();

  return res;
}

void Huffman::Encode(const string& text, base::BitString* bits) const {
  bits->clear();
  for (auto it = text.cbegin(); it != text.cend(); ++it) {
    bits->Append(encode_map_.at(*it));
  }

  // Null-terminate string
  bits->Append(encode_map_.at(0));
}

void Huffman::Encode(
    const void* text, int size, base::BitString* bits) const {
  const uint8_t* values_ptr = reinterpret_cast<const uint8_t*>(text);

  bits->clear();
  for (int i = 0; i < size; ++i) {
    bits->Append(encode_map_.at(values_ptr[i]));
  }
}

bool Huffman::Decode(const BitString& bits, void** data, int* size) const {
  vector<uint8_t> res = {};
  Node* node_iter = tree_;
  for (int i = 0; i < bits.size(); ++i) {
    if (bits.Get(i)) {
      node_iter = node_iter->get_right();
    } else {
      node_iter = node_iter->get_left();
    }

    if (node_iter->is_leaf()) {
      res.push_back(node_iter->get_symbol());
      node_iter = tree_;
    }
  }

  *size = res.size();
  *data = new uint8_t[*size];
  memcpy(*data, res.data(), *size);

  // Return true if and only if all bits contained usable information
  return (node_iter == tree_);
}

void Huffman::Serialize(void** buffer, int* size) const {
  // Several parts of this function depend upon
  // the histogram being the proper size.
  assert(histogram_.size() == base::kMaxByte);

  // TODO(hxtk): Use map of non-zero elements instead of histogram
  // if there are sufficiently few nonzero elements as to result in a smaller
  // header section. This format would use five-byte elements consisting of a
  // one-byte label and a four-byte quantity.
  // |205*5 > 1024|, so this format would only be used if there are 204 or fewer
  // non-zero elements.
  /*
  uint8_t count_nonzero = 0;
  for (auto it = histogram_.cbegin(); it != histogram_.cend(); ++it) {
    if (*it > 0) {
      // If the value is nonzero, increment the nonzero count.
      if(++count_nonzero > kBreakEvenHistogramSize) {
	// If we have reached breakeven size, there is no reason to continue
	// and furthermore it puts us at risk of overflow.
	break;
      }
    }
  }
  
  if (count_nonzero > kBreakEvenHistogramSize) {
  // END TODO
  */

  // Create a buffer large enough to hold a one-byte header
  // followed by the entire histogram.
  *size = (sizeof(histogram_.front())*histogram_.size()) + 1;
  uint8_t* working_buf = new uint8_t[*size];

  // This is a magic number indicating a big header in the format.
  // In future versions, there will be a small header format used, where this
  // byte will indicate the size of the header segment.
  *working_buf = 0;
  *buffer = working_buf;

  // NOTE: copy begins one byte after start of buffer
  // and copies one less than size of buffer to protect
  // the header byte at the front.
  memcpy(*buffer + 1, histogram_.data(), *size - 1);
}

bool Huffman::Unserialize(const void* bytes, int size) {
  if (size < Huffman::header_size(bytes)) {
    return false;
  }

  int num_entries = *(reinterpret_cast<const uint8_t*>(bytes));

  if (num_entries == 0) {
    histogram_.resize(base::kMaxByte);
    std::memcpy(histogram_.data(),
                bytes + 1,
                histogram_.size() * sizeof(histogram_.front()));
  } else {
    // TODO(hxtk): Corresponds to the smallheader format described above.
    return false;
  }
  this->BuildTree();
  return true;
}

string Huffman::ToString() const {
  if (tree_ == nullptr) return "";
  return ToString(tree_, 0);
}

string Huffman::ToString(Node* fakeroot, int depth) const {
  if (fakeroot->is_leaf()) {
    if (fakeroot->get_frequency() > 0) {
      return "(" + string({fakeroot->get_symbol()})
          + ", " + std::to_string(fakeroot->get_frequency()) +
          + ", " + std::to_string(depth) + ")";
    } else {
      return "";
    }
  } else {
    return ToString(fakeroot->get_left(), depth + 1)
      + " " + ToString(fakeroot->get_right(), depth + 1);
  }
}
}  // namespace huffman
}  // namespace compression
