#pragma once

#include <fstream>
#include <memory>

#include "LRTableBuilder.h"
#include "LRTableSerializer.h"
#include "ast/ASTContext.h"
#include "ast/Nodes.h"
#include "lexis/LexicalAnalyzer.h"

namespace Syntax {
class LRParser {
  std::vector<std::vector<Action>> actions_;
  std::vector<std::vector<size_t>> goto_;

  SourceManager& source_manager_;

 public:
  LRParser(const std::filesystem::path& path, SourceManager& manager)
      : source_manager_(manager) {
    std::ifstream is(path, std::ios_base::binary);
    std::tie(actions_, goto_) = LRTableSerializer::deserialize(is);
  }

  ASTContext parse(Lexis::LexicalAnalyzer& lexical_analyzer) const;
};
}  // namespace Syntax
