#pragma once

#include <variant>

#include "syntax/grammar/GrammarProduction.h"

namespace Syntax {

struct RejectAction {};
struct AcceptAction {};
struct ReduceAction {
  NonTerminal next;
  size_t remove_count;
  size_t production_index;
};
struct ShiftAction {
  size_t next_state;
};

using Action =
    std::variant<RejectAction, AcceptAction, ReduceAction, ShiftAction>;

struct SerializedAction {
  // 0 - RejectAction
  // 1 - AcceptAction
  // 2 - ReduceAction
  // 3 - ShiftAction
  uint8_t index;

  size_t field1;
  size_t field2;
  size_t field3;
};
}  // namespace Syntax
