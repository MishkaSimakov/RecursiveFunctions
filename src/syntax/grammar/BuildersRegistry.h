#pragma once

#include <functional>
#include <span>
#include <vector>

namespace Syntax {
struct BuilderFunction {
  size_t index;

  explicit BuilderFunction(size_t index) : index(index) {}
};

template <typename NodeT>
class BuildersRegistry {
 public:
  using BuilderFnT =
      std::function<std::unique_ptr<NodeT>(std::span<std::unique_ptr<NodeT>>)>;

 private:
  std::vector<BuilderFnT> builders_;

 public:
  BuilderFunction register_builder(BuilderFnT fn) {
    size_t index = builders_.size();
    builders_.emplace_back(std::move(fn));

    return BuilderFunction(index);
  }

  const BuilderFnT& operator[](size_t index) const { return builders_[index]; }
};
}  // namespace Syntax
