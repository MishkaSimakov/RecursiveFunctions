// #pragma once
// #include <unordered_map>
//
// #include "intermediate_representation/BasicBlock.h"
// #include "intermediate_representation/Instruction.h"
// #include "passes/Pass.h"
// #include "passes/analysis/dfa/ForwardDFA.h"
//
// namespace Passes {
// // TODO: in development (temporary abandoned because not very useful now)
// class ConstantPropagationPass
//     : public Pass,
//       public ForwardDFA<std::unordered_map<IR::Value, IR::Value>> {
//  protected:
//   DFAValueT meet(std::span<DFAValueT> parents) const override;
//
//   std::unordered_map<IR::Value, IR::Value> transfer(
//       const std::unordered_map<IR::Value, IR::Value>& before,
//       const IR::BaseInstruction& instruction) const override;
//
//   void init() override;
//
//   static IR::Value join_abstract_values(IR::Value, IR::Value);
//
//  public:
//   constexpr static auto kBottom = IR::Value(0, IR::ValueType::VIRTUAL_REGISTER);
//
//   explicit ConstantPropagationPass(PassManager& manager) : Pass(manager) {
//     throw std::runtime_error("Not implemented yet");
//   }
//
//   void apply() override;
// };
// }  // namespace Passes
