#include "FilesystemManager.h"

#include <iostream>
#include <random>

std::unique_ptr<FilesystemManager> FilesystemManager::instance_{nullptr};
std::mutex FilesystemManager::mutex_{};

FilesystemManager::FilesystemManager() {
  auto now = std::chrono::system_clock::now();
  auto duration = now.time_since_epoch();
  auto milliseconds =
      std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

  random_generator_.seed(milliseconds);
}

fs::path FilesystemManager::get_temporary_directory() {
  if (!temporary_directory_.empty()) {
    return temporary_directory_;
  }

  auto temp_dir = fs::temp_directory_path();

  temporary_directory_ = generate_unique_name(temp_dir);

  if (!fs::create_directory(temporary_directory_)) {
    throw std::runtime_error(
        "Failed to create temporary directory for compiler files");
  }

  return temporary_directory_;
}

fs::path FilesystemManager::generate_unique_name(fs::path directory,
                                                 const fs::path& extension,
                                                 size_t max_attempts) {
  size_t attempt = 0;

  while (true) {
    auto result = (directory / std::to_string(random_generator_()))
                      .replace_extension(extension);

    if (!fs::exists(result)) {
      return result;
    }

    if (attempt > max_attempts) {
      throw std::runtime_error(
          "Failed to create temporary directory for compiler needs");
    }

    ++attempt;
  }
}

FilesystemManager::Resource& FilesystemManager::get_or_create_resource(
    const fs::path& path) {
  for (auto& resource : loaded_resources_) {
    if (resource.path == path) {
      return resource;
    }
  }

  // create new resource
  Resource resource;
  resource.id = loaded_resources_.size();
  resource.path = path;

  return loaded_resources_.emplace_back(std::move(resource));
}

FilesystemManager& FilesystemManager::get_instance() {
  std::lock_guard guard(mutex_);

  if (instance_ == nullptr) {
    instance_ = std::unique_ptr<FilesystemManager>(new FilesystemManager());
  }

  return *instance_;
}
