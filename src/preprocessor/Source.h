#ifndef SOURCE_H
#define SOURCE_H

#include <string>
#include <vector>

using std::string, std::vector;

namespace Preprocessing {
class Source {
 public:
  virtual vector<string> get_text() const = 0;

  virtual ~Source() = default;
};
}  // namespace Preprocessing

#endif  // SOURCE_H
