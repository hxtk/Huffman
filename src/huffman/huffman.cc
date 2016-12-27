// Copyright: Peter Sanders. All rights reserved.
// Date: 2016-11-30

#include "huffman.h"

#include <cassert>
#include <cstdint>
#include <cstring>  

#include <string>   
#include <vector>
#include <queue>    
#include <unordered_map>

#include "node.h"
#include "comparator.h"

using std::string;
using std::vector;
using std::priority_queue;
using std::unordered_map;

/*void Huffman::BuildTree(const string& text) {
  histogram_ = vector<int>(kMaxChar, 0);
  for (int i = 0; i < text.size(); ++i) {
    // Characters used for text are signed.
    // As a result, they range over [-128, 127].
    // With an evil pointer hack, you can turn it into an unsigned byte
    // with the same bit pattern. This guarantees uniqueness and provides
    // valid indices to the |vector|.
    //
    // This is done by accessing the address and telling the program
    // to interpret the data at that address as an unsigned value
    // of the same size as the original value.
    //
    // NOTE: This will not work on systems which use bytes that are
    //       not 8 bits wide.
    ++histogram_.at(*(reinterpret_cast<const uint8_t*>(&text.at(i))));
  }

  priority_queue<Node*, vector<Node*>, Comparator> nodes;
  for (int i = 0; i < kMaxChar; ++i) {
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
}*/

void Huffman::BuildTree(const string& text) {
  this->BuildTree(
      reinterpret_cast<const unsigned char*>(text.c_str()), text.size());
}

void Huffman::BuildTree(const uint8_t*& text, int size) {
  histogram_ = vector<int>(kMaxChar, 0);
  for (int i = 0; i < size; ++i) {
    ++histogram_.at(text[i]);
  }
  this->BuildTree();
}

void Huffman::BuildTree() {
  // Create a node for each item in the histogram
  priority_queue<Node*, vector<Node*>, [&](Node* a, Node* b) {
    return *a < *b;
  }> nodes;
  for (int i = 0; i < kMaxChar; ++i) {
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
  return (tree_ != nullptr) && BuildMap(tree_, {});
}

bool Huffman::BuildMap(Node* fakeroot, vector<bool> bits) {
  if (fakeroot->is_leaf()) {
    encode_map_[fakeroot->get_symbol()] = bits;
    return true;
  }
  bool res = true;

  // Traverse into the left branch
  bits.push_back(false);
  res &= BuildMap(fakeroot->get_left(), bits);
  bits.pop_back();

  // Traverse into the right branch
  bits.push_back(true);
  res &= BuildMap(fakeroot->get_right(), bits);
  bits.pop_back();

  return res;
}

void Huffman::Encode(const string& text, vector<bool>* bits) const {
  bits->clear();
  for (auto it = text.cbegin(); it != text.cend(); ++it) {
    const vector<bool>& other = encode_map_.at(*it);
    bits->insert(bits->end(), other.cbegin(), other.cend());
  }
}

void Huffman::Encode(const char* text, int size, vector<bool>* bits) const {
  bits->clear();
  for (int i = 0; i < size; ++i) {
    const vector<bool>& other = encode_map_.at(text[i]);
    bits->insert(bits->end(), other.cbegin(), other.cend());
  }
}

string Huffman::Decode(const vector<bool>& bits) const {
  string res = "";
  Node* node_iter = tree_;
  for (auto it = bits.cbegin(); it != bits.cend(); ++it) {
    if (*it) {
      node_iter = node_iter->get_right();
    } else {
      node_iter = node_iter->get_left();
    }

    if (node_iter->is_leaf()) {
      res += node_iter->get_symbol();
      node_iter = tree_;
    }
  }
  return res;
}

char* Huffman::Serialize(char** buffer, char* size) const {
  // Several parts of this function depend upon
  // the histogram being the proper size.
  if (histogram_.size() != kMaxChar) return nullptr;
  
  int count_nonzero = 0;  // Number of non-zero values in histogram
  for (auto it = histogram_.cbegin(); it != histogram_.cend(); ++it) {
    if (*it > 0) ++count_nonzero;
  }

  uint8_t *res;
  if (count_nonzero > kBreakEvenHistogramSize) {
    *size = kMaxChar*sizeof(int) + 1;
    *buffer = new unsigned char[*size];

    // The 0th byte is the number of elements.
    // If the 0th byte is zero, this indicates that all elements are present
    *buffer[0] = 0;

    // There must be exactly enough space in the buffer
    // to hold the histogram and the header byte.
    assert(histogram_.size()*sizeof(int32_t) + 1 == num_bytes);

    // NOTE: begin one byte after start of buffer
    //       copy one fewer bytes than the size of the buffer
    std::memcpy(*buffer + 1,
                reinterpret_cast<const uint8_t*>(histogram_.data()),
                *size - 1);
  } else {
    *size = (count_nonzero * sizeof(int32_t)) + sizeof(uint8_t)
    *buffer = new unsigned char[*size];
    res[0] = count_nonzero;

    unsigned char* ptr = *buffer + 1;
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

void Huffman::Unserialize(const unsigned char* bytes) {
  if (bytes[0] == 0) {
    histogram_.resize(kMaxChar);
    std::memcpy(histogram_.data(),
                reinterpret_cast<const int32_t*>(bytes + 1),
                kMaxChar * sizeof(int32_t));
  } else {
    histogram_ = vector<int32_t>(256, 0);
    int num_entries = static_cast<int32_t>(bytes[0]);
    for (int i = 0; i < num_entries; ++i) {
      uint8_t index =
          *reinterpret_cast<const uint8_t*>(bytes + 1 + (i*kEntryWidth));
      int32_t value =
          *reinterpret_cast<const int32_t*>(bytes + 2 + (i*kEntryWidth));
      histogram_.at(index) = value;
    }
  }
  
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
