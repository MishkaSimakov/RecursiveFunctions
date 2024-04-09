#include <string>
#include <vector>

#include "compilation/Compiler.h"
#include "execution/BytecodeExecutor.h"
#include "lexis/LexicalAnalyzer.h"
#include "preprocessor/Preprocessor.h"
#include "syntax/RecursiveFunctionsSyntax.h"
#include "syntax/buffalo/SyntaxTreeBuilder.h"

int main() {
  // setup logger
  Logger::disable_category(Logger::Category::ALL);

  std::filesystem::path base_path =
      "/Users/mihailsimakov/Documents/Programs/CLionProjects/"
      "RecursiveFunctions/tests";

  Preprocessor preprocessor;
  preprocessor.add_file("arithmetics", base_path / "arithmetics.rec");
  preprocessor.set_main("arithmetics");
  string program_text = preprocessor.process();

  auto tokens = LexicalAnalyzer::get_tokens(program_text);

  auto syntax_tree = SyntaxTreeBuilder::build(
      tokens, RecursiveFunctionsSyntax::GetSyntax(),
      RecursiveFunctionsSyntax::RuleIdentifiers::PROGRAM);

  Compilation::Compiler compiler;
  auto bytecode = compiler.compile(*syntax_tree);

  BytecodeExecutor executor;
  ValueT result = executor.execute(bytecode);

  std::cout << "Executed successfully, result: " << result << endl;

  return 0;
}

// #include <iostream>
//
// #include "execution/BytecodeExecutor.h"
//
// using namespace std;
//
// int main() {
//   using namespace Compilation;
//
//   BytecodeExecutor executor;
//
//   // add function
//   // clang-format off
//   vector<Instruction> program = {
//     {InstructionType::LOAD_CONST, 100000},
//     {InstructionType::LOAD_CONST, 10012},
//     {InstructionType::CALL_FUNCTION, 14, 2},
//     {InstructionType::HALT},
//
//     // ADD FUNCTION, line 4
//     {InstructionType::LOAD, 0},
//     {InstructionType::JUMP_IF_ZERO, 11},
//     {InstructionType::DECREMENT, 0},
//     {InstructionType::LOAD, 1},
//     {InstructionType::CALL_FUNCTION, 4, 2},
//     {InstructionType::INCREMENT, 0},
//     {InstructionType::RETURN},
//     {InstructionType::POP},
//     {InstructionType::LOAD, 1},
//     {InstructionType::RETURN},
//
//     // FAST ADD FUNCTION, 14
//     {InstructionType::LOAD, 0},
//     {InstructionType::JUMP_IF_ZERO, 24},
//     {InstructionType::POP},
//     {InstructionType::LOAD, 1},
//     {InstructionType::LOAD, 0},
//     {InstructionType::INCREMENT, 1},
//     {InstructionType::DECREMENT, 0},
//     {InstructionType::JUMP_IF_NONZERO, 19},
//     {InstructionType::POP},
//     {InstructionType::RETURN},
//     {InstructionType::POP},
//     {InstructionType::LOAD, 1},
//     {InstructionType::RETURN}
//   };
//   // clang-format on
//
//   cout << executor.execute(program) << endl;
// }