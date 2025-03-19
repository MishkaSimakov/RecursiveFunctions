#pragma once

#include <ostream>
#include <sstream>

#include "CustomRegex.h"
#include "RegexNodes.h"

// TODO: make better parentesis
class RegexPrinter {
  const Regex& regex_;

 public:
  const RegexNode& get_root() const { return regex_.get_root(); }

  explicit RegexPrinter(const Regex& regex);
};

std::ostream& operator<<(std::ostream& os, const RegexPrinter& printer);
