#pragma once
#include <lexis/Charset.h>

#include <bitset>
#include <memory>

#define REGEX_CONST_NODE_VISITOR_VISIT(T) virtual void visit(const T& node) = 0;
#define REGEX_NODE_VISITOR_VISIT(T) virtual void visit(T& node) = 0;

#define REGEX_NODE_ACCEPT_VISITORS()                           \
  void accept(RegexConstNodeVisitor& visitor) const override { \
    visitor.visit(*this);                                      \
  }                                                            \
  void accept(RegexNodeVisitor& visitor) override { visitor.visit(*this); }

struct SymbolNode;
struct ConcatenationNode;
struct OrNode;
struct StarNode;
struct PlusNode;

struct RegexConstNodeVisitor {
  REGEX_CONST_NODE_VISITOR_VISIT(SymbolNode)
  REGEX_CONST_NODE_VISITOR_VISIT(ConcatenationNode)
  REGEX_CONST_NODE_VISITOR_VISIT(OrNode)
  REGEX_CONST_NODE_VISITOR_VISIT(StarNode)
  REGEX_CONST_NODE_VISITOR_VISIT(PlusNode)

  virtual ~RegexConstNodeVisitor() = default;
};

struct RegexNodeVisitor {
  REGEX_NODE_VISITOR_VISIT(SymbolNode)
  REGEX_NODE_VISITOR_VISIT(ConcatenationNode)
  REGEX_NODE_VISITOR_VISIT(OrNode)
  REGEX_NODE_VISITOR_VISIT(StarNode)
  REGEX_NODE_VISITOR_VISIT(PlusNode)

  virtual ~RegexNodeVisitor() = default;
};

struct RegexNode {
 protected:
  virtual bool equal_base(const RegexNode& node) const = 0;

 public:
  virtual ~RegexNode() = default;

  virtual std::unique_ptr<RegexNode> clone() const = 0;

  template <typename T>
  bool is_of_type() const {
    return typeid(T) == typeid(*this);
  }

  virtual void accept(RegexConstNodeVisitor&) const = 0;
  virtual void accept(RegexNodeVisitor&) = 0;

  bool operator==(const RegexNode& other) const {
    if (typeid(*this) != typeid(other)) {
      return false;
    }

    return equal_base(other);
  }
};

// match symbols in bitset
struct SymbolNode final : RegexNode {
  std::bitset<Charset::kCharactersCount> match;

  SymbolNode(std::bitset<Charset::kCharactersCount> match) : match(match) {}
  SymbolNode(size_t symbol) {
    match[symbol] = true;
  }

  std::unique_ptr<RegexNode> clone() const override {
    return std::make_unique<SymbolNode>(match);
  }

  REGEX_NODE_ACCEPT_VISITORS()

 protected:
  bool equal_base(const RegexNode& node) const override {
    const auto& other = static_cast<const SymbolNode&>(node);

    return other.match == match;
  }
};

// match left . right
struct ConcatenationNode final : RegexNode {
  std::unique_ptr<RegexNode> left;
  std::unique_ptr<RegexNode> right;

  ConcatenationNode(std::unique_ptr<RegexNode> left,
                    std::unique_ptr<RegexNode> right)
      : left(std::move(left)), right(std::move(right)) {}

  std::unique_ptr<RegexNode> clone() const override {
    return std::make_unique<ConcatenationNode>(left->clone(), right->clone());
  }

  REGEX_NODE_ACCEPT_VISITORS()

 protected:
  bool equal_base(const RegexNode& node) const override {
    const auto& other = static_cast<const ConcatenationNode&>(node);

    return *left == *other.left && *right == *other.right;
  }
};

// match left + right
struct OrNode final : RegexNode {
  OrNode(std::unique_ptr<RegexNode> left, std::unique_ptr<RegexNode> right)
      : left(std::move(left)), right(std::move(right)) {}

  std::unique_ptr<RegexNode> left;
  std::unique_ptr<RegexNode> right;

  std::unique_ptr<RegexNode> clone() const override {
    return std::make_unique<OrNode>(left->clone(), right->clone());
  }

  REGEX_NODE_ACCEPT_VISITORS()

 protected:
  bool equal_base(const RegexNode& node) const override {
    const auto& other = static_cast<const OrNode&>(node);

    return *left == *other.left && *right == *other.right;
  }
};

// match (child)*
struct StarNode final : RegexNode {
  std::unique_ptr<RegexNode> child;

  explicit StarNode(std::unique_ptr<RegexNode> child)
      : child(std::move(child)) {}

  std::unique_ptr<RegexNode> clone() const override {
    return std::make_unique<StarNode>(child->clone());
  }

  REGEX_NODE_ACCEPT_VISITORS()

 protected:
  bool equal_base(const RegexNode& node) const override {
    const auto& other = static_cast<const StarNode&>(node);

    return *child == *other.child;
  }
};

// match (child)+
struct PlusNode final : RegexNode {
  std::unique_ptr<RegexNode> child;

  explicit PlusNode(std::unique_ptr<RegexNode> child)
      : child(std::move(child)) {}

  std::unique_ptr<RegexNode> clone() const override {
    return std::make_unique<PlusNode>(child->clone());
  }

  REGEX_NODE_ACCEPT_VISITORS()

 protected:
  bool equal_base(const RegexNode& node) const override {
    const auto& other = static_cast<const PlusNode&>(node);

    return *child == *other.child;
  }
};
