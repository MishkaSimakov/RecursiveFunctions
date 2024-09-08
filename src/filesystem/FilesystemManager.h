#pragma once
#include <filesystem>
#include <fstream>
#include <random>
#include <vector>

namespace fs = std::filesystem;

template <typename T>
concept Printer = requires(T printer, std::ostream os) {
  os << printer;
  { printer.extension() } -> std::convertible_to<fs::path>;
};

class StringPrinter {
 private:
  std::string_view string_;
  fs::path extension_;

 public:
  StringPrinter(std::string_view string, fs::path extension)
      : string_(string), extension_(std::move(extension)) {}

  std::string_view get_string() const { return string_; }

  fs::path extension() const { return extension_; }
};

inline std::ostream& operator<<(std::ostream& os, const StringPrinter& sp) {
  os << sp.get_string();
  return os;
}

class FilesystemManager {
 public:
  struct Resource {
    size_t id{0};
    fs::path path{};
    std::string content{};
  };

 private:
  FilesystemManager();
  FilesystemManager(FilesystemManager&) = delete;
  FilesystemManager& operator=(const FilesystemManager&) = delete;

  static std::unique_ptr<FilesystemManager> instance_;
  static std::mutex mutex_;

  fs::path temporary_directory_{};
  std::vector<Resource> loaded_resources_{};
  std::mt19937 random_generator_;

  fs::path get_temporary_directory();
  fs::path generate_unique_name(fs::path directory,
                                const fs::path& extension = fs::path(),
                                size_t max_attempts = 100);

  Resource& get_or_create_resource(const fs::path& path);

 public:
  static FilesystemManager& get_instance();

  template <Printer T>
  const Resource& save_temporary(T&& printer) {
    fs::path temp_dir = get_temporary_directory();
    fs::path extension = printer.extension();
    fs::path resulting_path = generate_unique_name(temp_dir, extension);

    return save_to_file(resulting_path, std::forward<T>(printer));
  }

  template <Printer T>
  const Resource& save_to_file(fs::path path, T&& printer) {
    fs::path extension = printer.extension();
    fs::path resulting_path = path.replace_extension(extension);

    std::ofstream os(resulting_path);
    os << printer;

    return get_or_create_resource(resulting_path);
  }
};
