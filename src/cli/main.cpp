#include <argparse/argparse.hpp>

#include "ArgumentsReader.h"
#include "ast/ASTPrinter.h"
#include "compilation/TeaFrontend.h"
#include "errors/ExceptionsHandler.h"
#include "lexis/LexicalAnalyzer.h"
#include "syntax/lr/LRParser.h"
#include "utils/Constants.h"

const bool Constants::is_installed_build = BUILD_FOR_INSTALLATION;

namespace Cli {
namespace fs = std::filesystem;

class Main {
 public:
  static int main(int argc, char* argv[]) {
    return ExceptionsHandler::execute([argc, argv] {
      auto config = ArgumentsReader::read(argc, argv);

      auto front = Front::TeaFrontend(std::move(config));
      front.compile();

      // Front::GlobalContext context;
      // auto& source_manager = context.source_manager;
      //
      // Lexis::LexicalAnalyzer lexical_analyzer(Constants::lexis_filepath);
      // auto parser = Syntax::LRParser(Constants::grammar_filepath);
      //
      // auto& [name, path] = *arguments.sources.begin();
      // SourceView source_view = source_manager.load(path);
      // lexical_analyzer.set_source_view(source_view);
      //
      // auto& module_context = context.add_module(name);
      //
      // try {
      //   parser.parse(lexical_analyzer, module_context, source_view);
      // } catch (Syntax::ParserException exception) {
      //   for (const auto& [position, error] : exception.get_errors()) {
      //     source_manager.add_annotation(position, error);
      //   }
      //
      //   source_manager.print_annotations(std::cout);
      //   throw;
      // }
      //
      // if (arguments.dump_ast) {
      //   Front::ASTPrinter(module_context, std::cout).print();
      // } else {
      //   try {
      //     Interpretation::ASTInterpreter(module_context).interpret();
      //   } catch (Interpretation::InterpreterException exception) {
      //     source_manager.add_annotation(exception.get_range(),
      //                                   exception.what());
      //     source_manager.print_annotations(std::cout);
      //     throw;
      //   }
      // }
    });
  }
};
}  // namespace Cli

int main(int argc, char* argv[]) { return Cli::Main::main(argc, argv); }
