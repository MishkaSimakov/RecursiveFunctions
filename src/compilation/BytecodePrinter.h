#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "Instructions.h"

using std::vector, std::string;

class BytecodePrinter {
 public:
  static void print(const vector<Compilation::Instruction>& instructions) {
    for (size_t line_id = 0; line_id < instructions.size(); ++line_id) {
      std::cout << line_id << ":\t" << instructions[line_id] << std::endl;
    }
  }
};
