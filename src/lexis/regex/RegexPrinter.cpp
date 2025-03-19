#include "RegexPrinter.h"

#include "RegexNodes.h"
#include "lexis/Charset.h"

namespace {
struct RegexPrinterVisitor final : RegexConstNodeVisitor {
 public:
  std::ostream& os;

  explicit RegexPrinterVisitor(std::ostream& os) : os(os) {}

  void visit(const SymbolNode& node) override {
    os << "[";
    auto groups = group_symbols(node.match);

    for (auto [from, to] : groups) {
      os << print_symbol(from);
      if (from != to) {
        os << "-" << print_symbol(to);
      }
    }
    os << "]";
  }
  void visit(const ConcatenationNode& node) override {
    node.left->accept(*this);
    node.right->accept(*this);
  }
  void visit(const OrNode& node) override {
    os << "(";
    node.left->accept(*this);
    os << " | ";
    node.right->accept(*this);
    os << ")";
  }
  void visit(const StarNode& node) override {
    os << "(";
    node.child->accept(*this);
    os << ")*";
  }
  void visit(const PlusNode& node) override {
    os << "(";
    node.child->accept(*this);
    os << ")+";
  }
};
}  // namespace

RegexPrinter::RegexPrinter(const Regex& regex) : regex_(regex) {}

std::ostream& operator<<(std::ostream& os, const RegexPrinter& printer) {
  auto print_visitor = RegexPrinterVisitor(os);
  printer.get_root().accept(print_visitor);
  return os;
}
