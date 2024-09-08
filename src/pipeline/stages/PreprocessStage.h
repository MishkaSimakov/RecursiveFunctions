#pragma once
#include "ReadCliArgumentsStage.h"

class PreprocessStage {
  constexpr static auto kIncludeNamePathDelimiter = ":";

 public:
  using input = CompilerSettings;
  using output = std::string;
  constexpr static auto name = "preprocess";

  std::string apply(CompilerSettings settings);
};
