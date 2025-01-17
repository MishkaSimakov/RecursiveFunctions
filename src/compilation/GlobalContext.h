#pragma once
#include <deque>
#include <unordered_map>

#include "compilation/ModuleContext.h"
#include "sources/SourceManager.h"
#include "types/TypesStorage.h"
#include "compilation/StringId.h"

struct GlobalContext {
  SourceManager source_manager;
  TypesStorage types_storage;

  std::unordered_map<std::string_view, ModuleContext*> module_names_mapping;
  std::deque<ModuleContext> modules;
  std::vector<std::string> strings_table;

  ModuleContext& add_module(std::string_view name) {
    size_t index = modules.size();
    auto& module = modules.emplace_back(index);
    module_names_mapping.emplace(name, &module);
    return module;
  }

  StringId add_string(std::string_view string) {
    // TODO: maybe use hash table?
    for (size_t i = 0; i < strings_table.size(); ++i) {
      if (strings_table[i] == string) {
        return StringId(i);
      }
    }

    strings_table.emplace_back(string);
    return StringId(strings_table.size() - 1);
  }

  std::string_view get_string(StringId index) const {
    return strings_table[index.id_];
  }
};
