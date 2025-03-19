#pragma once
#include <string>
#include <string_view>

#include "RegexNodes.h"
#include "RegexParser.h"

class Regex {
  std::unique_ptr<RegexNode> root_;

 public:
  explicit Regex(std::unique_ptr<RegexNode> node) : root_(std::move(node)) {}

  explicit Regex(std::string_view string) : root_(RegexParser::parse(string)) {}
  explicit Regex(char symbol) : Regex(std::string(1, symbol)) {}

  Regex(const Regex& other) : root_(other.root_->clone()) {}
  Regex(Regex&& other) = default;

  Regex& operator=(const Regex& other) {
    if (this != &other) {
      root_ = other.root_->clone();
    }

    return *this;
  }
  Regex& operator=(Regex&&) = default;

  const RegexNode& get_root() const { return *root_; }

  Regex& iterate() {
    root_ = std::make_unique<StarNode>(std::move(root_));
    return *this;
  }

  Regex& operator+=(const Regex& other) {
    if (this != &other) {
      root_ = std::make_unique<OrNode>(std::move(root_), other.root_->clone());
    }

    return *this;
  }

  Regex& operator+=(Regex&& other) {
    if (this != &other) {
      root_ =
          std::make_unique<OrNode>(std::move(root_), std::move(other.root_));
    }

    return *this;
  }

  Regex& operator*=(const Regex& other) {
    if (this != &other) {
      root_ = std::make_unique<ConcatenationNode>(std::move(root_),
                                                  other.root_->clone());
    }

    return *this;
  }

  std::string to_string() const;
};

inline Regex operator+(const Regex& left, const Regex& right) {
  Regex copy = left;
  copy += right;
  return copy;
}

inline Regex operator*(const Regex& left, const Regex& right) {
  Regex copy = left;
  copy *= right;
  return copy;
}
