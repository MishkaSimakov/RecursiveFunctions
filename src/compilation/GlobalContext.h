#pragma once
#include <deque>
#include <unordered_map>

#include "compilation/ModuleContext.h"
#include "compilation/StringId.h"
#include "sources/SourceManager.h"
#include "types/TypesStorage.h"

namespace Front {
struct GlobalContext {
  SourceManager source_manager;

  std::unordered_map<std::string_view, ModuleContext*> module_names_mapping;
  std::deque<ModuleContext> modules;

  ModuleContext& add_module(std::string_view name) {
    size_t index = modules.size();
    auto& module = modules.emplace_back(index);
    module_names_mapping.emplace(name, &module);
    return module;
  }
};
}  // namespace Front
