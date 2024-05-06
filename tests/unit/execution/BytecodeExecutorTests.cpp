#include <gtest/gtest.h>

#include <vector>

#include "compilation/Instructions.h"
#include "execution/BytecodeExecutor.h"

using Compilation::Instruction, Compilation::InstructionType;
using std::vector;

TEST(BytecodeExecutorTests, test_simple_program) {
  vector<Instruction> instructions = {
      {InstructionType::LOAD_CONST, 10}, {InstructionType::LOAD_CONST, 20},
      {InstructionType::DECREMENT, 0},   {InstructionType::DECREMENT, 1},
      {InstructionType::POP, 1},         {InstructionType::HALT, 0}};

  BytecodeExecutor executor;
  auto result = executor.execute(instructions);

  ASSERT_EQ(result.as_value(), 19);
}

TEST(BytecodeExecutorTests, test_it_throws_when_iterations_limit_reached) {
  vector<Instruction> instructions = {{InstructionType::LOAD_CONST, 2},
                                      {InstructionType::LOAD_CONST, 1},
                                      {InstructionType::POP, 1},
                                      {InstructionType::HALT, 0}};

  BytecodeExecutor executor;
  executor.set_iterations_limit(3);
  ASSERT_THROW({ executor.execute(instructions); }, std::runtime_error);

  BytecodeExecutor second_executor;
  second_executor.set_iterations_limit(4);
  ASSERT_NO_THROW({ second_executor.execute(instructions); });
}
