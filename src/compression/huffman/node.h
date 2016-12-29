// Copyright: Peter Sanders. All rights reserved.
// Date: 2016-11-30
//
// This class defines a node of a Huffman Coding Tree

#ifndef HUFFMAN_NODE_H_
#define HUFFMAN_NODE_H_

namespace huffman {
class Node {
 public:
  //////////////////////////////////////////////////////////////////////////////
  // Constants
  
  // The null character is used as a placeholder
  // when no printable character has been set yet
  constexpr static char kDummySymbol = '\0';

  // Since frequency is at least |0|, |-1| serves to
  // indicate no valid frequency has been given yet.
  constexpr static int kDummyFrequency = -1;

  //////////////////////////////////////////////////////////////////////////////
  // Factory Functions

  // This builds a non-leaf node, which shall have no symbol and two children.
  // The frequency is the sum of the frequency of the two given children.
  // A pointer to the constructed Node is returned.
  static Node* BuildBranch(Node* left, Node* right) {
    Node* res = new Node();
    res->Init(left, right, left->frequency_ + right->frequency_);
    return res;
  }

  // This builds a leaf node, which shall have a symbol but no children.
  // It returns a pointer to the constructed Node.
  static Node* BuildLeaf(char symbol, int frequency) {
    Node* res = new Node();
    res->InitLeaf(symbol, frequency);
    return res;
  }

  //////////////////////////////////////////////////////////////////////////////
  // Destructor
  ~Node() {
    delete left_;
    delete right_;
  }

  //////////////////////////////////////////////////////////////////////////////
  // Other functions
  
  bool operator>(const Node& rhs) const {
    return (this->frequency_ > rhs.frequency_);
  }

  Node* get_left() const {
    return left_;
  }

  Node* get_right() const {
    return right_;
  }

  char get_symbol() const {
    return symbol_;
  }

  int get_frequency() const {
    return frequency_;
  }

  int is_leaf() const {
    return (left_ == nullptr && right_ == nullptr);
  }

 private:
  Node() {}
  
  // A Node contains a symbol if and only if it is a leaf Node.
  // A Leaf Node by definition must have no children.
  bool InitLeaf(char symbol, int frequency) {
    symbol_ = symbol;
    frequency_ = frequency;
    return true;
  }

  // If a Node is not a leaf node, the symbol is left at the default
  // All other values are used.
  bool Init(Node* left, Node* right, int frequency) {
    left_ = left;
    right_ = right;
    frequency_ = frequency;
    return true;
  }
  
  char symbol_ = kDummySymbol;
  int frequency_ = kDummyFrequency;

  Node* left_ = nullptr;
  Node* right_ = nullptr;
};  // class Node
}  // namespace huffman
 
#endif  // HUFFMAN_NODE_H_
