// Copyright: Peter Sanders. All rights reserved.
// Date: 2016-11-30

#include "huffman/huffman.h"

#include <cassert>
#include <cstdint>
#include <cstring>  

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <queue>    
#include <unordered_map>

#include "huffman/node.h"
#include "huffman/comparator.h"

#include "base/bitstring.h"

using std::string;
using std::vector;
using std::priority_queue;
using std::unordered_map;

using base::BitString;

using std::cout;
using std::endl;

namespace huffman {
void Huffman::BuildTree(const void* text, int size) {
  uint8_t* values_ptr = reinterpret_cast<uint8_t*>(text);

  histogram_ = vector<int>(base::kMaxByte, 0);
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
}

void Huffman::Encode(
    const uint8_t* text, int size, base::BitString* bits) const {
  bits->clear();
  for (int i = 0; i < size; ++i) {
    bits->Append(encode_map_.at(text[i]));
  }
}

vector<uint8_t> Huffman::Decode(const BitString& bits) const {
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
  return res;
}

void Huffman::Serialize(void** buffer, int* size) const {
  // Several parts of this function depend upon
  // the histogram being the proper size.
  assert(histogram_.size() == base::kMaxByte);
  
  uint8_t count_nonzero = 0;  // Number of non-zero values in histogram
  for (auto it = histogram_.cbegin(); it != histogram_.cend(); ++it) {
    // No need to continue running calculations if the outcome is determined
    // Also, allowing it to exceed this would permit overflow if all values
    // were non-zero.
    if (count_nonzero > kBreakEvenHistogramSize) {
	break;
    }
    if (*it > 0) {
      ++count_nonzero;
    }
  }

  if (count_nonzero > kBreakEvenHistogramSize) {
    *size = base::kMaxByte*sizeof(int) + sizeof(uint8_t);
    *buffer = new uint8_t[*size];

    // The 0th byte is the number of elements.
    // If the 0th byte is zero, this indicates that all elements are present
    (*buffer)[0] = 0;

    // There must be exactly enough space in the buffer
    // to hold the histogram and the header byte.
    assert(histogram_.size()*sizeof(histogram_.front()) + 1 == *size);

    // NOTE: begin one byte after start of buffer;
    //       copy one fewer bytes than the size of the buffer
    //       This is because the heder takes up the first byte.
    std::memcpy(*buffer + 1,
                reinterpret_cast<const uint8_t*>(histogram_.data()),
                *size - 1);
  } else {
    *size = (static_cast<uint32_t>(count_nonzero) * kEntryWidth)
      + sizeof(count_nonzero);
    *buffer = new uint8_t[*size];
    (*buffer)[0] = count_nonzero;

    uint8_t* ptr = *buffer + 1;
    for (int i = 0; i < histogram_.size(); ++i) {
      if (histogram_.at(i) > 0) {
        int* int_ptr = reinterpret_cast<int*>(ptr + 1);
        
        *ptr = i;
        *int_ptr = histogram_.at(i);

        ptr += kEntryWidth;
      }
    }
  }
}

bool Huffman::Unserialize(const void* bytes, int size) {
  if (size < Huffman::get_header_size(bytes)) {
    return false;
  }
  assert(size >= Huffman::get_header_size(bytes));
  if (bytes[0] == 0) {
    histogram_.resize(base::kMaxByte);
    std::memcpy(histogram_.data(),
                bytes + 1,
                histogram_.size() * sizeof(histogram_.front()));
  } else {
    histogram_ = vector<int32_t>(256, 0);
    int num_entries = static_cast<int32_t>(bytes[0]);
    for (int i = 0; i < num_entries*kEntryWidth; i += kEntryWidth) {
      uint8_t index =
          *reinterpret_cast<const uint8_t*>(bytes + 1 + (i*kEntryWidth));
      int32_t value =
          *reinterpret_cast<const int32_t*>(bytes + 2 + (i*kEntryWidth));
      histogram_.at(index) = value;
    }
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
