#pragma once
#include <cmath>
#include <iostream>
#include <vector>

class DynamicBitset {
  using BlockT = int64_t;
  constexpr static size_t kBlockSize = 64;

  size_t size_;
  BlockT* data_;

  constexpr static size_t get_blocks_count(size_t size) {
    // we divide and ceil
    return (size + kBlockSize - 1) / kBlockSize;
  }

  static size_t get_remainder(size_t index) {
    return (kBlockSize - index % kBlockSize) % kBlockSize;
  }

  void size_guard(const DynamicBitset& other) const {
    if (other.size_ != size_) {
      throw std::runtime_error("Sizes must be equal to perform operations");
    }
  }

  struct Proxy {
    BlockT* data;
    size_t offset;

    operator bool() const {
      return (*data & static_cast<BlockT>(1) << offset) != 0;
    }

    Proxy& operator=(bool value) {
      if (value) {
        *data |= static_cast<BlockT>(1) << offset;
      } else {
        *data &= ~(static_cast<BlockT>(1) << offset);
      }

      return *this;
    }
  };

  void swap(DynamicBitset& other) {
    std::swap(data_, other.data_);
    std::swap(size_, other.size_);
  }

 public:
  explicit DynamicBitset(size_t size)
      : size_(size), data_(new BlockT[get_blocks_count(size_)]{}) {}

  DynamicBitset(const DynamicBitset& other) : DynamicBitset(other.size_) {
    memcpy(data_, other.data_, get_blocks_count(size_));
  }

  DynamicBitset(DynamicBitset&& other)
      : size_(other.size_), data_(other.data_) {
    other.data_ = nullptr;
    other.size_ = 0;
  }

  DynamicBitset& operator=(const DynamicBitset& other) {
    if (this != &other) {
      DynamicBitset(other).swap(*this);
    }

    return *this;
  }

  DynamicBitset& operator=(DynamicBitset&& other) {
    DynamicBitset(std::move(other)).swap(*this);

    return *this;
  }

  DynamicBitset& operator&=(const DynamicBitset& other) {
    size_guard(other);

    for (size_t i = 0; i < get_blocks_count(size_); ++i) {
      data_[i] &= other.data_[i];
    }

    return *this;
  }

  DynamicBitset& operator|=(const DynamicBitset& other) {
    size_guard(other);

    for (size_t i = 0; i < get_blocks_count(size_); ++i) {
      data_[i] |= other.data_[i];
    }

    return *this;
  }

  DynamicBitset operator|(const DynamicBitset& right) const {
    DynamicBitset copy = *this;
    copy |= right;
    return copy;
  }

  DynamicBitset operator&(const DynamicBitset& right) const {
    DynamicBitset copy = *this;
    copy &= right;
    return copy;
  }

  DynamicBitset& flip() {
    size_t blocks_count = get_blocks_count(size_);
    for (size_t i = 0; i < blocks_count; ++i) {
      data_[i] = ~data_[i];
    }

    data_[blocks_count - 1] &=
        (static_cast<BlockT>(1) << (size_ % kBlockSize)) - 1;

    return *this;
  }

  bool operator==(const DynamicBitset& other) const {
    if (size_ != other.size_) {
      return false;
    }

    for (size_t i = 0; i < get_blocks_count(size_); ++i) {
      if (data_[i] != other.data_[i]) {
        return false;
      }
    }

    return true;
  }

  Proxy operator[](size_t index) {
    if (index >= size_) {
      throw std::runtime_error("Out of boundaries access");
    }

    return Proxy{data_ + index / kBlockSize, index % kBlockSize};
  }

  bool operator[](size_t index) const {
    if (index >= size_) {
      throw std::runtime_error("Out of boundaries access");
    }

    return Proxy{data_ + index / kBlockSize, index % kBlockSize};
  }

  size_t size() const { return size_; }

  ~DynamicBitset() { delete[] data_; }
};

std::ostream& operator<<(std::ostream& os, const DynamicBitset& dbs) {
  for (size_t i = 0; i < dbs.size(); ++i) {
    os << dbs[i];
  }

  return os;
}
