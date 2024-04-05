#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H
#include <filesystem>
#include <fstream>
#include <iostream>
#include <list>
#include <regex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using std::string, std::unordered_map, std::vector, std::unordered_set,
    std::pair, std::list;

struct FileFragment {
  bool is_include;
  string value;

  FileFragment(bool is_include, string value)
      : is_include(is_include), value(std::move(value)) {}
};

using File = list<FileFragment>;

class Preprocessor {
  const static vector<char> kIgnoredCharacters;
  const static std::regex kIncludeRegex;

  unordered_map<string, std::filesystem::path> files_;
  string main_filename_ = "main";

  static bool is_include(const string& line) {
    return std::regex_search(line, kIncludeRegex);
  }

  static string extract_include_name(const string& include) {
    std::smatch matches;
    std::regex_search(include, matches, kIncludeRegex);

    return matches[1].str();
  }

  static bool should_preserve(const string& line) {
    return !line.starts_with("#");
  }

  static void remove_unnecessary_symbols(string& line) {
    std::erase_if(line, [](char symbol) { return std::isspace(symbol) != 0; });
  }

  auto compact_files() {
    string buffer;
    unordered_map<string, File> result;

    for (auto& [name, path] : files_) {
      std::ifstream file_stream(path);

      while (std::getline(file_stream, buffer)) {
        auto& file = result[name];

        if (is_include(buffer)) {
          file.emplace_back(true, extract_include_name(buffer));
          continue;
        }

        if (!should_preserve(buffer)) {
          continue;
        }

        remove_unnecessary_symbols(buffer);

        if (!file.empty() && !file.back().is_include) {
          file.back().value += buffer;
        } else {
          file.emplace_back(false, std::move(buffer));
        }
      }
    }

    return result;
  }

  static void substitute_includes(
      const string& filename, unordered_map<string, File>& compacted_files) {
    File& file = compacted_files[filename];

    for (auto position = file.begin(); position != file.end();) {
      if (!position->is_include) {
        ++position;
        continue;
      }

      string& include_filename = position->value;

      if (!compacted_files[include_filename].empty()) {
        substitute_includes(include_filename, compacted_files);
        file.splice(position, compacted_files[include_filename]);
      }

      position = file.erase(position);
    }
  }

 public:
  void add_file(const string& name, const std::filesystem::path& path) {
    files_.emplace(name, path);
  }

  void set_main(const string& name) { main_filename_ = name; }

  string process() {
    if (!files_.contains(main_filename_)) {
      throw std::runtime_error("Main file called " + main_filename_ +
                               " not found");
    }

    auto compacted_files = compact_files();
    substitute_includes(main_filename_, compacted_files);

    string result;

    for (const auto& fragment : compacted_files[main_filename_]) {
      result += fragment.value;
    }

    return result;
  }
};

const vector<char> Preprocessor::kIgnoredCharacters = {' ',  '\f', '\n',
                                                       '\r', '\t', '\v'};
const std::regex Preprocessor::kIncludeRegex(R"(#include\s*\"(.*)\")");

#endif  // PREPROCESSOR_H
