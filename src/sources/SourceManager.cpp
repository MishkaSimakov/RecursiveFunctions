#include "SourceManager.h"

#include <fcntl.h>
#include <fmt/format.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <algorithm>
#include <fstream>

SourceLocation SourceManager::load(const std::filesystem::path& path) {
  int fd = open(path.c_str(), O_RDWR);

  if (fd == -1) {
    throw std::runtime_error("Failed to open source file.");
  }

  struct stat statbuf;
  if (fstat(fd, &statbuf) == -1) {
    throw std::runtime_error("Failed to read file stat.");
  }

  off_t file_size = statbuf.st_size;
  void* mapped_ptr = mmap(nullptr, file_size, PROT_READ, MAP_PRIVATE, fd, 0);

  if (mapped_ptr == MAP_FAILED) {
    throw std::runtime_error("Failed to load source file.");
  }

  char* begin = static_cast<char*>(mapped_ptr);

  size_t file_id = loaded_.size();
  loaded_.emplace_back(begin, file_size, path);

  return SourceLocation(file_id, 0);
}

std::string_view SourceManager::get_file_view(SourceLocation location) const {
  if (location.file_id >= loaded_.size()) {
    return {};
  }

  auto [begin, size, _] = loaded_[location.file_id];
  if (size <= location.pos_id) {
    return {};
  }

  std::string_view view(begin, size);
  view.remove_prefix(location.pos_id);
  return view;
}

std::string_view SourceManager::get_file_view(SourceRange source_range) const {
  auto [begin, end] = source_range;
  if (begin.pos_id > end.pos_id || begin.file_id != end.file_id ||
      begin.file_id >= loaded_.size()) {
    return {};
  }

  auto [file_begin, size, _] = loaded_[begin.file_id];
  if (size <= end.pos_id) {
    return {};
  }

  return {file_begin + begin.pos_id, file_begin + end.pos_id + 1};
}

std::string_view SourceManager::get_line_view(SourceLocation location) const {
  auto [begin, size, _] = loaded_[location.file_id];
  const char* line_begin = begin;

  for (size_t i = 0; i < location.pos_id; ++i) {
    if (begin[i] == '\n') {
      line_begin = begin + i + 1;
    }
  }

  const char* line_end = line_begin;
  while (line_end < begin + size && *line_end != '\n') {
    ++line_end;
  }

  return {line_begin, line_end};
}

size_t SourceManager::get_line_id(SourceLocation location) const {
  auto begin = loaded_[location.file_id].begin;

  return std::count(begin, begin + location.pos_id, '\n');
}

void SourceManager::add_annotation(SourceLocation location,
                                   std::string_view text) {
  annotations_.emplace_back(location, text);
}

void SourceManager::print_annotations(std::ostream& os) {
  for (const SourceAnnotation& annotation : annotations_) {
    auto [file_id, position] = annotation.position;
    auto [begin, size, path] = loaded_[file_id];
    size_t line_id = get_line_id(annotation.position);

    os << fmt::format("In file {:?} on line {}:\n", path.c_str(), line_id);
    os << "\t" << get_line_view(annotation.position) << "\n";
    os << "\t" << annotation.value << "\n";
  }
}

SourceManager::~SourceManager() {
  for (auto& [begin, size, _] : loaded_) {
    munmap(begin, size);
  }
}
