#include "CustomRegex.h"

#include <iostream>
#include <list>

#include "RegexPrinter.h"

std::string Regex::to_string() const {
  std::stringstream result;
  result << RegexPrinter(*this);

  return result.str();
}
