#pragma once

#include <lexis/LexicalAnalyzer.h>
#include <syntax/lr/LRParser.h>
#include <utils/Constants.h>

#include <argparse/argparse.hpp>

#include "ArgumentsReader.h"
#include "ast/ASTPrinter.h"
#include "compilation/TeaFrontend.h"
#include "errors/ExceptionsHandler.h"
#include "interpretation/ASTInterpreter.h"

namespace Cli {
namespace fs = std::filesystem;

class Main {
 public:
  static int main(int argc, char* argv[]) {
    return ExceptionsHandler::execute([argc, argv] {
      auto arguments = ArgumentsReader::read(argc, argv);

      if (arguments.sources.size() != 1) {
        throw std::runtime_error("Only one source file must be provided.");
      }

      Front::GlobalContext context;
      auto& source_manager = context.source_manager;

      Lexis::LexicalAnalyzer lexical_analyzer(Constants::lexis_filepath);
      auto parser = Syntax::LRParser(Constants::grammar_filepath, context);

      auto& [name, path] = *arguments.sources.begin();
      SourceView source_view = source_manager.load(path);
      lexical_analyzer.set_source_view(source_view);

      auto& module_context = context.add_module(name);

      try {
        parser.parse(lexical_analyzer, module_context.id);
      } catch (Syntax::ParserException exception) {
        for (const auto& [position, error] : exception.get_errors()) {
          source_manager.add_annotation(position, error);
        }

        source_manager.print_annotations(std::cout);
        throw;
      }

      if (arguments.dump_ast) {
        Front::ASTPrinter(context, module_context, std::cout).print();
      } else {
        Interpretation::ASTInterpreter(context, module_context)
            .traverse();
      }
    });
  }
};
}  // namespace Cli
