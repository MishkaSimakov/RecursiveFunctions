#pragma once

#include <deque>
#include <filesystem>
#include <string>

struct SourceRange {
  size_t index;
  std::string::const_iterator begin;
  std::string::const_iterator end;

  std::string_view value() const { return {begin, end}; }
};

class SourcesLoader {
 public:
  struct FileSource {
    size_t index;
    std::string import_name;
    std::filesystem::path path;
    std::string content;
  };

 private:
  std::deque<FileSource> sources_;

 public:
  const FileSource& add_source(std::string import_name,
                               std::filesystem::path path);

  const FileSource& get_source(size_t index) const { return sources_[index]; }
};
