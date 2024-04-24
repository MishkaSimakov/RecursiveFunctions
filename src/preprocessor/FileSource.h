#ifndef FILESOURCE_H
#define FILESOURCE_H

#include <filesystem>
#include <fstream>
#include <optional>
#include <vector>

#include "Source.h"

using std::optional, std::vector;

namespace Preprocessing {
class FileSource final : public Source {
  std::filesystem::path path_;
  mutable optional<vector<string>> cached_;

 public:
  explicit FileSource(std::filesystem::path path) : path_(std::move(path)) {}

  vector<string> get_text() const override {
    if (!cached_) {
      std::ifstream file_stream(path_);
      cached_ = vector<string>();

      while (cached_->emplace_back(),
             std::getline(file_stream, cached_->back())) {
      }
    }

    return *cached_;
  }
};
}  // namespace Preprocessing

#endif  // FILESOURCE_H
