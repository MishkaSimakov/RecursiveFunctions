#include "SourceManager.h"

#include <fcntl.h>
#include <fmt/color.h>
#include <fmt/format.h>
#include <sys/errno.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <algorithm>
#include <fstream>

SourceView SourceManager::load(const std::filesystem::path& path) {
  int fd = open(path.c_str(), O_RDWR);

  if (fd == -1) {
    throw std::runtime_error("Failed to open source file.");
  }

  struct stat statbuf;
  if (fstat(fd, &statbuf) == -1) {
    throw std::runtime_error("Failed to read file stat.");
  }

  off_t file_size = statbuf.st_size;
  void* mapped_ptr;
  if (file_size == 0) {
    // mmap doesn't accept file_size = 0 so we should treat this case separately
    // MAP_ANONYMOUS is guaranteed to fill memory with zeros
    mapped_ptr =
        mmap(nullptr, 1, PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  } else {
    mapped_ptr = mmap(nullptr, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
  }

  if (mapped_ptr == MAP_FAILED) {
    throw std::runtime_error(
        fmt::format("Failed to load source file: {}", strerror(errno)));
  }

  char* begin = static_cast<char*>(mapped_ptr);

  size_t file_id = loaded_.size();
  loaded_.emplace_back(begin, file_size, path);

  return get_file_view(SourceLocation(file_id, 0));
}

SourceView SourceManager::load_text(std::string_view text) {
  char* loaded_text = new char[text.size() + 1];
  std::ranges::copy(text, loaded_text);

  size_t file_id = loaded_.size();
  loaded_.emplace_back(loaded_text, text.size(), std::filesystem::path{});

  return get_file_view(SourceLocation(file_id, 0));
}

SourceView SourceManager::get_file_view(SourceLocation location) const {
  if (location.file_id >= loaded_.size()) {
    throw std::runtime_error("Incorrect source location.");
  }

  auto [begin, size, _] = loaded_[location.file_id];
  if (size < location.pos_id) {
    throw std::runtime_error("Incorrect source location.");
  }

  std::string_view view(begin, size);
  view.remove_prefix(location.pos_id);
  return SourceView(view, location);
}

SourceView SourceManager::get_file_view(SourceRange source_range) const {
  auto [begin, end] = source_range;

  if (begin.pos_id > end.pos_id || begin.file_id != end.file_id ||
      begin.file_id >= loaded_.size()) {
    throw std::runtime_error("Incorrect source range.");
  }

  auto [file_begin, size, _] = loaded_[begin.file_id];
  if (size < end.pos_id) {
    throw std::runtime_error("Incorrect source range.");
  }

  std::string_view content(file_begin + begin.pos_id, file_begin + end.pos_id);
  return SourceView(content, source_range.begin);
}

SourceManager::LineInfo SourceManager::get_line_info(
    SourceLocation location) const {
  auto [begin, size, _] = loaded_[location.file_id];
  const char* line_begin = begin;
  size_t line_index = 0;

  for (size_t i = 0; i < location.pos_id; ++i) {
    if (begin[i] == '\n') {
      line_begin = begin + i + 1;
      ++line_index;
    }
  }

  const char* line_end = line_begin;
  while (line_end < begin + size && *line_end != '\n') {
    ++line_end;
  }

  auto view = std::string_view(line_begin, line_end);
  size_t offset = location.pos_id - (line_begin - begin);
  return {view, line_index, offset};
}

void SourceManager::add_annotation(SourceRange range, std::string_view text) {
  annotations_.emplace_back(range, text);
}

void SourceManager::print_annotations(std::ostream& os) {
  for (const SourceAnnotation& annotation : annotations_) {
    auto [file_id, position] = annotation.range.begin;
    auto [begin, size, path] = loaded_[file_id];
    auto [line_view, start_index, start_offset] =
        get_line_info(annotation.range.begin);

    auto [_, end_index, end_offset] = get_line_info(annotation.range.end);

    if (start_index != end_index) {
      // TODO: implement multi-line annotations
      throw std::runtime_error("Annotations must be on one line.");
    }

    // split line view to emphasize wrong part with red
    auto before_error_view = line_view.substr(0, start_offset);
    auto error_view = line_view.substr(start_offset, end_offset - start_offset);
    std::string emphasized_error =
        fmt::format(fg(fmt::color::orange), "{}", error_view);
    auto after_error_view = line_view.substr(end_offset);

    os << fmt::format("{}:{}:{}:\n", path.c_str(), start_index + 1,
                      start_offset + 1);
    os << before_error_view << emphasized_error << after_error_view << "\n";
    os << std::string(start_offset, ' ') << "`-" << annotation.value
       << std::endl;
  }
}

SourceManager::~SourceManager() {
  for (auto& [begin, size, path] : loaded_) {
    if (path.empty()) {
      delete[] begin;
    } else {
      munmap(begin, size);
    }
  }
}
