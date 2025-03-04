#pragma once

#include <filesystem>
#include <vector>

#include "SourceLocation.h"

struct SourceAnnotation {
  SourceRange range;
  std::string value;

  SourceAnnotation(SourceRange range, std::string_view value)
      : range(range), value(value) {}
};

struct LoadedFileInfo {
  char* begin;
  size_t size;

  // for pure texts path is empty
  std::filesystem::path path;

  LoadedFileInfo(char* begin, size_t size, std::filesystem::path path)
      : begin(begin), size(size), path(std::move(path)) {}
};

class SourceView {
  std::string_view string_view_;
  SourceLocation begin_;

  template <bool ShouldCheck>
  size_t get_offset(SourceLocation location) const {
    if constexpr (ShouldCheck) {
      if (location.file_id != begin_.file_id) {
        throw std::runtime_error(
            "Locations must be from the same file when accessing SourceView.");
      }
      if (location.pos_id < begin_.pos_id ||
          begin_.pos_id + string_view_.size() <= location.pos_id) {
        throw std::runtime_error("Location is outside of SourceView range.");
      }
    }

    return location.pos_id - begin_.pos_id;
  }

 public:
  SourceView() : begin_(0, 0) {}

  SourceView(std::string_view string_view, SourceLocation begin)
      : string_view_(string_view), begin_(begin) {}

  const char& operator[](SourceLocation location) const {
    return string_view_[get_offset<false>(location)];
  }

  const char& at(SourceLocation location) const {
    return string_view_[get_offset<true>(location)];
  }

  std::string_view string_view() const { return string_view_; }
  std::string_view string_view(SourceRange range) const {
    size_t begin_offset = get_offset<false>(range.begin);
    size_t end_offset = get_offset<false>(range.end);

    return string_view_.substr(begin_offset, end_offset - begin_offset);
  }

  SourceLocation begin_location() const { return begin_; }

  SourceLocation end_location() const {
    SourceLocation end = begin_;
    end.pos_id += string_view_.size();
    return end;
  }
};

class SourceManager {
  std::vector<LoadedFileInfo> loaded_;
  std::vector<SourceAnnotation> annotations_;

 public:
  SourceView load(const std::filesystem::path& path);
  SourceView load_text(std::string_view text);

  SourceView get_file_view(SourceLocation location) const;
  SourceView get_file_view(SourceRange source_range) const;

  struct LineInfo {
    std::string_view view;
    size_t index;
    size_t offset;
  };
  LineInfo get_line_info(SourceLocation location) const;

  void add_annotation(SourceRange range, std::string_view text);
  void print_annotations(std::ostream& os);

  ~SourceManager();
};
