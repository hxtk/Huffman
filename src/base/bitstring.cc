// Copyright: Peter Sanders. All rights reserved.
// Date: 2016-12-27

#include "base/bitstring.h"

#include <cassert>
#include <cstring>
#include <cstdint>

#include <vector>

namespace base {
BitString& BitString::operator=(const BitString& rhs) {
  size_ = rhs.size_;
  memcpy(bytes_.data(), rhs.bytes_.data(), bytes_.size());
  return *this;
}

bool BitString::Set(uint32_t index, bool value) {
  assert(index < size_);

  if (value) {  // Bitmask to set the appropriate bit
    bytes_.at(index / kByteBits) |= (kMaxBit >> (index % kByteBits));
  } else {  // Bitmask to unset the appropriate bit
    bytes_.at(index / kByteBits) &= ~(kMaxBit >> (index % kByteBits));
  }

  return value;  // Return the input value to allow chaining
}

bool BitString::Get(uint32_t index) const {
  assert(index < size_);

  uint8_t byte = bytes_.at(index / kByteBits);
  byte <<= (index % kByteBits);

  return ((byte & kMaxBit) != 0);
}

void BitString::Append(bool value) {
  // First, we handle the edge cases where we are creating a new byte.
  if (size_ % 8 == 0) {
    if (value) {
      bytes_.push_back(kMaxBit);
    } else {
      bytes_.push_back(0);
    }
  }

  // Next increment the size.
  ++size_;

  // The general case is a special case of `Set()` using the index one greater
  // than the previous max index.
  this->Set(size_ - 1, value);
}


// TODO(hxtk): optimize edge case where memory copy is possible
void BitString::Append(const BitString& bits) {
  for (int i = 0; i < bits.size(); ++i) {
    this->Append(bits.Get(i));
  }
}

void BitString::PopBack() {
  // First we handle the edge case that only one bit is stored in the last byte
  if (size_ % kByteBits == 1) {
    bytes_.pop_back();
  }

  // Bits after and including the |size_|th bits are not well-defined and may
  // take any value, so they are not altered. The size is simply reduced.
  --size_;
}

void BitString::Serialize(void** buffer, int* size) const {
  *size = sizeof(size_) + bytes_.size();
  *buffer = new uint8_t[*size];

  uint32_t* int_ptr = reinterpret_cast<uint32_t*>(*buffer);
  *int_ptr = size_;

  memcpy(*buffer + sizeof(size_), bytes_.data(), bytes_.size());
}

bool BitString::Unserialize(const void* input, int size) {
  this->clear();

  // There must be at least enough space for the size header
  if (size < sizeof(size_))
    return false;
  
  size_ = *(reinterpret_cast<const uint32_t*>(input));

  // Since we cannot allocate fractions of bytes, a trailing partially-filled
  // byte must be considered full. As such we use a least-integer function
  // which is less trivial than the greatest-integer function without the use
  // of math libraries.
  uint32_t container_size = (size_ % 8 == 0) ? (size_ / 8) : ((size_ / 8) + 1);

  // Now the size is well-defined, we can make the final size test.
  if (size < container_size)
    return false;

  // Now that the container has the proper size
  // copy the data segment of the buffer into it
  bytes_.resize(container_size);
  memcpy(bytes_.data(), input + sizeof(size_), bytes_.size());

  return true;
}
}  // namespace base
