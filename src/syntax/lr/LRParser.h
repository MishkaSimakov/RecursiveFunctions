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
class ParserException final : public std::runtime_error {
  std::vector<std::pair<SourceRange, std::string>> errors_;

 public:
  explicit ParserException(
      std::vector<std::pair<SourceRange, std::string>> errors)
      : std::runtime_error("Parser error."), errors_(std::move(errors)) {}

  const auto& get_errors() const { return errors_; }
};

class LRParser {
  const std::vector<std::vector<Action>> actions_;
  const std::vector<std::vector<size_t>> goto_;

  // This private constructor required to initalize two constant fields
  // from one function return value
  explicit LRParser(
      std::pair<LRTableSerializer::ActionsTableT, LRTableSerializer::GotoTableT>
          tables)
      : actions_(std::move(tables.first)), goto_(std::move(tables.second)) {}

 public:
  explicit LRParser(const std::filesystem::path& path)
      : LRParser([&path] {
          std::ifstream is(path, std::ios_base::binary);

          if (!is) {
            throw std::runtime_error("Failed to open lr-table file.");
          }

          return LRTableSerializer::deserialize(is);
        }()) {}

  void parse(Lexis::LexicalAnalyzer& lexical_analyzer,
             Front::ModuleContext& context, SourceView source) const;
};
}  // namespace Syntax
