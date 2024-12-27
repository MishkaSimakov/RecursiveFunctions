#include "LRTableSerializer.h"

#include "utils/TupleUtils.h"

namespace Syntax {
void LRTableSerializer::serialize(std::ostream& os,
                                  const ActionsTableT& actions_table,
                                  const GotoTableT& goto_table) {
  // file format:
  // 1. states count (size_t)
  // 2. non-terms count (size_t)
  // 3. actions table
  // 4. goto table

  size_t states_count = actions_table.size();
  size_t nonterms_count = goto_table.front().size();

  // compact nonterms
  auto write_bytes = [&os](size_t value) {
    os.write(reinterpret_cast<const char*>(&value), sizeof(size_t));
  };

  write_bytes(states_count);
  write_bytes(nonterms_count);

  for (const auto& state_actions : actions_table) {
    for (Action action : state_actions) {
      write_bytes(action.index());
      std::visit(Overloaded{[](AcceptAction) {}, [](RejectAction) {},
                            [&write_bytes](ShiftAction shift) {
                              write_bytes(shift.next_state);
                            },
                            [&write_bytes](ReduceAction reduce) {
                              write_bytes(reduce.next.get_id());
                              write_bytes(reduce.remove_count);
                              write_bytes(reduce.builder.index);
                            }},
                 action);
    }
  }

  for (const auto& state_gotos : goto_table) {
    for (size_t next_state : state_gotos) {
      write_bytes(next_state);
    }
  }
}

std::pair<LRTableSerializer::ActionsTableT, LRTableSerializer::GotoTableT>
LRTableSerializer::deserialize(std::istream& is) {
  auto read_bytes = [&is]() {
    size_t result;
    is.read(reinterpret_cast<char*>(&result), sizeof(size_t));
    return result;
  };

  size_t states_count = read_bytes();
  size_t nonterms_count = read_bytes();
  size_t tokens_count = Lexis::TokenType::count;

  ActionsTableT actions_table(states_count);

  for (size_t i = 0; i < states_count; ++i) {
    actions_table[i].resize(tokens_count);
    for (size_t j = 0; j < tokens_count; ++j) {
      size_t index = read_bytes();
      switch (index) {
        case variant_type_index_v<AcceptAction, Action>:
          actions_table[i][j] = AcceptAction();
          break;
        case variant_type_index_v<RejectAction, Action>:
          actions_table[i][j] = RejectAction();
          break;
        case variant_type_index_v<ShiftAction, Action>:
          actions_table[i][j] = ShiftAction{read_bytes()};
          break;
        case variant_type_index_v<ReduceAction, Action>:
          actions_table[i][j] =
              ReduceAction{NonTerminal{read_bytes()}, read_bytes(),
                           BuilderFunction{read_bytes()}};
          break;
      }
    }
  }

  GotoTableT goto_table(states_count);

  for (size_t i = 0; i < states_count; ++i) {
    goto_table[i].resize(nonterms_count);

    for (size_t j = 0; j < nonterms_count; ++j) {
      goto_table[i][j] = read_bytes();
    }
  }

  return {std::move(actions_table), std::move(goto_table)};
}
}  // namespace Syntax
