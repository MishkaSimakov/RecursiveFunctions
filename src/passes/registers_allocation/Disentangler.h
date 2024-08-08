#pragma once

#include <vector>
#include <deque>

#include "intermediate_representation/Value.h"

namespace Passes {
class Disentangler {
  struct PermutationNode {
    IR::Value value;
    PermutationNode* next{nullptr};
    PermutationNode* prev{nullptr};

    PermutationNode() = default;

    explicit PermutationNode(IR::Value value) : value(value) {}
  };

  void disentangle_chains(std::vector<std::pair<IR::Value, IR::Value>>&, std::deque<PermutationNode>&);
 public:
  std::vector<std::pair<IR::Value, IR::Value>> disentangle(
      const std::vector<std::pair<IR::Value, IR::Value>>&, IR::Value);
};
}  // namespace Passes
