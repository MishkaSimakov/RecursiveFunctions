#include "Type.h"

namespace Front {

bool Type::is_unit() const {
  return get_kind() == Kind::TUPLE &&
         static_cast<const TupleType*>(this)->elements.empty();
}

}  // namespace Front
