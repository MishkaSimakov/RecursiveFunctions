#pragma once

#include <lexis/LexicalAnalyzer.h>
#include <syntax/lr/LRParser.h>
#include <utils/Constants.h>

#include <argparse/argparse.hpp>

#include "ArgumentsReader.h"
#include "ast/ASTPrinter.h"
#include "compilation/TeaFrontend.h"
#include "errors/ExceptionsHandler.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

using namespace llvm;

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

      Front::TeaFrontend(arguments.sources).compile();

      auto context = std::make_unique<LLVMContext>();
      auto module = std::make_unique<Module>("sample tea module", *context);

      // Create a new builder for the module.
      auto builder = std::make_unique<IRBuilder<>>(*context);

      // make sample main function
      std::vector<Type*> types;
      FunctionType* FT =
          FunctionType::get(Type::getDoubleTy(*context), types, false);

      Function* F =
          Function::Create(FT, Function::ExternalLinkage, "main", module.get());

      BasicBlock* BB = BasicBlock::Create(*context, "entry", F);
      builder->SetInsertPoint(BB);

      Value* return_value = ConstantInt::get(*context, APInt(64, 123));
      builder->CreateRet(return_value);
      verifyFunction(*F);

      module->print(outs(), nullptr);

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
