#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H
#include <list>
#include <regex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Exceptions.h"
#include "Source.h"

using std::string, std::unordered_map, std::vector, std::unordered_set,
    std::pair, std::list;

namespace Preprocessing {
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

  unordered_map<string, std::unique_ptr<Source>> sources_;
  string main_source_ = "main";

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

  static bool is_symbol_space_preserving(char symbol) {
    return std::isdigit(symbol) != 0 || std::isalpha(symbol) != 0 ||
           symbol == '_';
  }

  static void remove_unnecessary_symbols(string& line) {
    // remove all space symbols
    // with one exception:
    // space cannot (and would not) be removed if two separate identifiers or
    // numbers will become one

    for (auto itr = line.begin(); itr != line.end();) {
      bool is_space = std::isspace(*itr) != 0;
      bool not_on_edge = itr != line.begin() && std::next(itr) != line.end();

      if (is_space && not_on_edge &&
          (!is_symbol_space_preserving(*std::prev(itr)) ||
           !is_symbol_space_preserving(*std::next(itr)))) {
        itr = line.erase(itr);
      } else {
        ++itr;
      }
    }
  }

  auto process_files() {
    unordered_map<string, File> result;

    for (auto& [name, source] : sources_) {
      auto source_text = source->get_text();
      auto& result_text = result[name];

      for (auto& line : source_text) {
        if (is_include(line)) {
          result_text.emplace_back(true, extract_include_name(line));
          continue;
        }

        if (!should_preserve(line)) {
          continue;
        }

        remove_unnecessary_symbols(line);

        if (!result_text.empty() && !result_text.back().is_include) {
          result_text.back().value += line;
        } else {
          result_text.emplace_back(false, std::move(line));
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

      if (!compacted_files.contains(include_filename)) {
        throw IncludeSourceNotFoundException(filename, include_filename);
      }

      if (!compacted_files[include_filename].empty()) {
        substitute_includes(include_filename, compacted_files);
        file.splice(position, compacted_files[include_filename]);
      }

      position = file.erase(position);
    }
  }

 public:
  template <typename T>
    requires std::is_base_of_v<Source, T>
  void add_source(const string& name, auto&&... args) {
    Logger::preprocessor(LogLevel::DEBUG, "added source of type",
                         typeid(T).name(), "with name:", name);

    sources_.emplace(
        name, std::make_unique<T>(std::forward<decltype(args)>(args)...));
  }

  void set_main_source(const string& name) {
    Logger::preprocessor(LogLevel::DEBUG, "set main source name to:", name);

    main_source_ = name;
  }

  string process() {
    Logger::preprocessor(LogLevel::INFO, "start preprocessing files");

    if (!sources_.contains(main_source_)) {
      throw MainSourceNotFoundException(main_source_);
    }

    auto compacted_files = process_files();
    substitute_includes(main_source_, compacted_files);

    string result;

    for (const auto& fragment : compacted_files[main_source_]) {
      result += fragment.value;
    }

    Logger::preprocessor(LogLevel::INFO, "successfully preprocessed all files");
    Logger::preprocessor(LogLevel::DEBUG, "resulting program:");
    Logger::preprocessor(LogLevel::DEBUG, result);

    return result;
  }
};

inline const vector<char> Preprocessor::kIgnoredCharacters = {' ',  '\f', '\n',
                                                              '\r', '\t', '\v'};
inline const std::regex Preprocessor::kIncludeRegex(R"(#include\s*\"(.*)\")");
}  // namespace Preprocessing

#endif  // PREPROCESSOR_H
