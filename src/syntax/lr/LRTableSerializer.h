#pragma once

#include <iostream>
#include <vector>

#include "Action.h"

namespace Syntax {
class LRTableSerializer {
 public:
  static void serialize(std::ostream& os,
                        const std::vector<std::vector<Action>>& actions_table,
                        const std::vector<std::vector<size_t>>& goto_table);

  static SerializedAction serialize_action(Action action);

  static Action deserialize_action(SerializedAction action);
};
}  // namespace Syntax
