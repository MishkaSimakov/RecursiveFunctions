#pragma once

#include <filesystem>
#include <vector>

#include "SourceLocation.h"

struct SourceAnnotation {
  SourceLocation position;
  std::string value;

  SourceAnnotation(SourceLocation position, std::string_view value)
      : position(position), value(value) {}
};

struct LoadedFileInfo {
  char* begin;
  size_t size;

  // for pure texts path is empty
  std::filesystem::path path;

  LoadedFileInfo(char* begin, size_t size, std::filesystem::path path)
      : begin(begin), size(size), path(std::move(path)) {}
};

class SourceManager {
  std::vector<LoadedFileInfo> loaded_;
  std::vector<SourceAnnotation> annotations_;

 public:
  SourceLocation load(const std::filesystem::path& path);
  SourceLocation load_text(std::string_view text);

  std::string_view get_file_view(SourceLocation location) const;
  std::string_view get_file_view(SourceRange source_range) const;

  struct LineInfo {
    std::string_view view;
    size_t index;
    size_t offset;
  };
  LineInfo get_line_info(SourceLocation location) const;

  void add_annotation(SourceLocation location, std::string_view text);
  void print_annotations(std::ostream& os);

  ~SourceManager();
};
