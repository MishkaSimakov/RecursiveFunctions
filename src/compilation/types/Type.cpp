#include "Type.h"

namespace Front {

bool Type::is_unit() const {
  return get_kind() == Kind::TUPLE &&
         static_cast<const TupleType*>(this)->get_elements().empty();
}

}  // namespace Front
