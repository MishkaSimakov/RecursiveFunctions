#pragma once

#include <string>
#include <vector>

#include "Source.h"

using std::vector, std::string;

namespace Preprocessing {
class TextSource final : public Source {
  vector<string> text_;

 public:
  explicit TextSource(vector<string> text) : text_(std::move(text)) {}

  vector<string> get_text() const override { return text_; }
};
}  // namespace Preprocessing
