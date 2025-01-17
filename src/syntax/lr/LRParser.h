#pragma once

#include <fstream>
#include <memory>

#include "LRTableBuilder.h"
#include "LRTableSerializer.h"
#include "ast/ASTBuildContext.h"
#include "ast/Nodes.h"
#include "compilation/GlobalContext.h"
#include "compilation/ModuleContext.h"
#include "compilation/types/TypesStorage.h"
#include "lexis/LexicalAnalyzer.h"

namespace Syntax {
class LRParser {
  std::vector<std::vector<Action>> actions_;
  std::vector<std::vector<size_t>> goto_;

  GlobalContext& context_;

 public:
  LRParser(const std::filesystem::path& path, GlobalContext& context)
      : context_(context) {
    std::ifstream is(path, std::ios_base::binary);
    std::tie(actions_, goto_) = LRTableSerializer::deserialize(is);
  }

  void parse(Lexis::LexicalAnalyzer& lexical_analyzer, size_t module_id) const;
};
}  // namespace Syntax
