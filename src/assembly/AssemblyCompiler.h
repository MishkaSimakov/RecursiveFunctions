#pragma once

#include <functional>
#include <list>
#include <string>
#include <typeindex>
#include <unordered_map>

#include "AssemblyInstruction.h"
#include "intermediate_representation/BasicBlock.h"
#include "intermediate_representation/Instruction.h"

namespace Assembly {
class AssemblyCompiler {
  static std::string mangle_name(const std::string& name) { return "_" + name; }

  static auto decorate_function(const std::string& name,
                                std::list<AssemblyInstruction> body,
                                bool is_leaf = false);

  using AssemblyStorage =
      std::unordered_map<const IR::BasicBlock*, std::list<AssemblyInstruction>>;
  using AssemblyGeneratorT =
      std::function<void(const IR::Instruction&, const IR::BasicBlock*, std::list<AssemblyInstruction>&)>;

  constexpr static std::unordered_map<std::type_index, AssemblyGeneratorT>
      generators_;
  AssemblyStorage storage_;

  void compile_basic_block(const IR::Function&, const IR::BasicBlock*);
  void compile_function(const IR::Function&);

  std::vector<const IR::BasicBlock*> arrange_blocks(const IR::Function&);

 public:
  AssemblyCompiler() = default;
};


}  // namespace Assembly
