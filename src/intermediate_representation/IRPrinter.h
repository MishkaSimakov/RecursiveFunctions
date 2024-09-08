#pragma once

#include <iostream>
#include <filesystem>

#include "Program.h"

namespace IR {
struct IRPrinter {
  Program& program_;

  static std::filesystem::path extension() {
    return ".recir";
  }
};

std::ostream& operator<<(std::ostream& os, const IRPrinter& printer);
} // IR
