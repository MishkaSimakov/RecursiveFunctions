#pragma once
#include <map>

#include "compilation/ModuleContext.h"
#include "sources/SourceManager.h"

namespace Front {
struct GlobalContext {
 private:
  std::map<std::string, ModuleContext, std::less<>> modules_;

 public:
  SourceManager source_manager;

  ModuleContext& add_module(std::string name) {
    auto [itr, was_emplaced] = modules_.emplace(name, ModuleContext{});
    itr->second.name = name;
    return itr->second;
  }

  bool has_module(std::string_view name) const {
    return modules_.contains(name);
  }

  ModuleContext& get_module(std::string_view name) {
    auto itr = modules_.find(name);
    return itr->second;
  }

  const auto& get_modules() const { return modules_; }
};
}  // namespace Front
