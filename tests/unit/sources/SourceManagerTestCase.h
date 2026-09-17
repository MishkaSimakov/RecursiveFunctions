#pragma once

#include <fmt/format.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <vector>

#include "sources/SourceManager.h"

class SourceManagerTestCase : public ::testing::Test {
 private:
  std::vector<std::filesystem::path> created_;

 protected:
  Front::SourceConfig create_source(std::string_view content,
                                    std::filesystem::perms permissions,
                                    bool is_std = false) {
    auto name = ::testing::UnitTest::GetInstance()->current_test_info()->name();
    std::filesystem::path path =
        std::filesystem::temp_directory_path() /
        fmt::format("tlang_{}_{}.tea", name, created_.size());

    std::ofstream(path) << content;
    std::filesystem::permissions(path, permissions);
    created_.push_back(path);

    return Front::SourceConfig{.path = std::move(path), .is_std = is_std};
  }

  void TearDown() override {
    for (const std::filesystem::path& path : created_) {
      std::filesystem::remove(path);
    }
  }
};
