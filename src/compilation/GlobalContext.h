#pragma once
#include <map>

#include "compilation/ModuleContext.h"
#include "sources/SourceManager.h"

namespace Front {
struct GlobalContext {
 private:
  StringPool strings_;
  std::map<std::string, ModuleContext, std::less<>> modules_;

 public:
  SourceManager source_manager;

  ModuleContext& add_module(std::string name) {
    auto [itr, _] = modules_.emplace(name, strings_);
    itr->second.name = name;
    return itr->second;
  }

  bool has_module(std::string_view name) const {
    return modules_.contains(name);
  }

  ModuleContext& get_module(std::string_view name) {
    auto itr = modules_.find(name);

    if (itr == modules_.end()) {
      throw std::runtime_error(fmt::format(
          "Unknown module {:?} requested in ModuleContext::get_module.", name));
    }

    return itr->second;
  }

  const auto& get_modules() const { return modules_; }
};
}  // namespace Front
