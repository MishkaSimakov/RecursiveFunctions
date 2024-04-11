#include <string>
#include <vector>

#include "compilation/BytecodePrinter.h"
#include "compilation/Compiler.h"
#include "execution/BytecodeExecutor.h"
#include "lexis/LexicalAnalyzer.h"
#include "preprocessor/Preprocessor.h"
#include "syntax/RecursiveFunctionsSyntax.h"
#include "syntax/buffalo/SyntaxTreeBuilder.h"

// int main() {
//   // setup logger
//   Logger::disable_category(Logger::Category::ALL);
//
//   std::filesystem::path base_path =
//       "/Users/mihailsimakov/Documents/Programs/CLionProjects/"
//       "RecursiveFunctions/tests";
//
//   Preprocessor preprocessor;
//   // preprocessor.add_file("arithmetics", base_path / "arithmetics.rec");
//   preprocessor.add_file("test", base_path / "test.rec");
//   preprocessor.set_main("test");
//   string program_text = preprocessor.process();
//
//   cout << program_text << endl;
//
//   auto tokens = LexicalAnalyzer::get_tokens(program_text);
//
//   auto syntax_tree = SyntaxTreeBuilder::build(
//       tokens, RecursiveFunctionsSyntax::GetSyntax(),
//       RecursiveFunctionsSyntax::RuleIdentifiers::PROGRAM);
//
//   Compilation::BytecodeCompiler compiler;
//   auto bytecode = compiler.compile(*syntax_tree);
//
//   BytecodePrinter::print(bytecode);
//
//   BytecodeExecutor executor;
//   ValueT result = executor.execute(bytecode);
//
//   std::cout << "Executed successfully, result: " << result << endl;
//
//   return 0;
// }

#include <iostream>

#include "execution/BytecodeExecutor.h"

using namespace std;

int main() {
  using namespace Compilation;

  BytecodeExecutor executor;

  // add function
  // clang-format off
  vector<Instruction> program = {
    {InstructionType::LOAD_CALL, 18},
    {InstructionType::LOAD_CONST, 1000},
    {InstructionType::LOAD_CONST, 1000},
    {InstructionType::CALL_FUNCTION},
    {InstructionType::HALT},

    // ADD FUNCTION, line 5
    {InstructionType::LOAD, 0},
    {InstructionType::JUMP_IF_ZERO, 15},
    {InstructionType::POP},
    {InstructionType::LOAD_CALL, 5},
    {InstructionType::LOAD, 0},
    {InstructionType::DECREMENT, 0},
    {InstructionType::LOAD, 1},
    {InstructionType::CALL_FUNCTION},
    {InstructionType::INCREMENT, 0},
    {InstructionType::RETURN},
    {InstructionType::POP},
    {InstructionType::LOAD, 1},
    {InstructionType::RETURN},

    // FAST ADD FUNCTION, 18
    {InstructionType::LOAD, 0},
    {InstructionType::JUMP_IF_ZERO, 29},
    {InstructionType::POP},
    {InstructionType::LOAD_CALL, 18},
    {InstructionType::LOAD, 1},
    {InstructionType::LOAD, 0},
    {InstructionType::INCREMENT, 1},
    {InstructionType::DECREMENT, 0},
    {InstructionType::JUMP_IF_NONZERO, 24},
    {InstructionType::POP},
    {InstructionType::RETURN},
    {InstructionType::POP},
    {InstructionType::LOAD, 1},
    {InstructionType::RETURN}
  };
  // clang-format on

  Logger::disable_category(Logger::EXECUTION);
  cout << executor.execute(program).get_value() << endl;
}