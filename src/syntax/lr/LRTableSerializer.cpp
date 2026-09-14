#include "LRTableSerializer.h"

#include <fmt/ostream.h>

#include <cassert>

#include "utils/IncFilesGeneration.h"
#include "utils/TupleUtils.h"

namespace Syntax {
void LRTableSerializer::serialize(
    std::ostream& os, const std::vector<std::vector<Action>>& actions_table,
    const std::vector<std::vector<size_t>>& goto_table) {
  // file format:
  // 1. states count (size_t)
  // 2. non-terms count (size_t)
  // 3. actions table
  // 4. goto table

  write_header(os);

  fmt::println(os, "constexpr size_t states_count = {};", actions_table.size());
  fmt::println(os, "constexpr size_t nonterms_count = {};",
               goto_table.front().size());

  // serialize each action into 4 size_t
  os << "constexpr SerializedAction actions_table[] = {\n";
  for (const auto& state_actions : actions_table) {
    assert(state_actions.size() == Lexis::TokenType::count);

    for (Action action : state_actions) {
      const auto serialized = serialize_action(action);

      os << fmt::format("SerializedAction({}, {}, {}, {}), ", serialized.index,
                        serialized.field1, serialized.field2,
                        serialized.field3);
    }
    os << "\n";
  }
  os << "};\n\n";

  os << "const size_t goto_table[] = {\n";
  for (const auto& state_gotos : goto_table) {
    assert(state_gotos.size() == goto_table.front().size());

    for (size_t next_state : state_gotos) {
      os << next_state << ", ";
    }
    os << "\n";
  }
  os << "};\n\n";
}

SerializedAction LRTableSerializer::serialize_action(Action action) {
  SerializedAction result{};

  std::visit(Overloaded{
                 [&result](RejectAction) { result.index = 0; },
                 [&result](AcceptAction) { result.index = 1; },
                 [&result](ReduceAction reduce) {
                   result.index = 2;
                   result.field1 = reduce.next.get_id();
                   result.field2 = reduce.remove_count;
                   result.field3 = reduce.production_index;
                 },
                 [&result](ShiftAction shift) {
                   result.index = 3;
                   result.field1 = shift.next_state;
                 },
             },
             action);

  return result;
}

Action LRTableSerializer::deserialize_action(SerializedAction action) {
  switch (action.index) {
    case 0:
      return RejectAction();
    case 1:
      return AcceptAction();
    case 2:
      return ReduceAction(NonTerminal(action.field1), action.field2,
                          action.field3);
    case 3:
      return ShiftAction(action.field1);
    default:
      throw std::runtime_error("Error during deserialization.");
  }
}

}  // namespace Syntax
