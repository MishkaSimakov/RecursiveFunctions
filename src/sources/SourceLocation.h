#pragma once

struct SourceLocation {
  uint32_t file_id;
  uint32_t pos_id;

  SourceLocation() : file_id(0), pos_id(0) {}
  SourceLocation(uint32_t file_id, uint32_t pos_id)
      : file_id(file_id), pos_id(pos_id) {}
};

struct SourceRange {
  SourceLocation begin;
  SourceLocation end;

  SourceRange() = default;
  SourceRange(SourceLocation begin, SourceLocation end)
      : begin(begin), end(end) {}

  static SourceRange merge(SourceRange first, SourceRange second) {
    return SourceRange(first.begin, second.end);
  }
};
