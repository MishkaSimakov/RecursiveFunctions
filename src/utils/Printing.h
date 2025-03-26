#pragma once

#include <iostream>

class TreePrinter {
  struct TextNode {
    std::string value;
    std::vector<std::unique_ptr<TextNode>> children;

    TextNode(std::string value) : value(std::move(value)) {}
  };

  std::unique_ptr<TextNode> root_;
  std::vector<TextNode*> nodes_stack_;
  std::ostream& os_;

  void print_recursive(const TextNode& node,
                       std::vector<std::string>& prefixes) {
    for (auto& str : prefixes) {
      os_ << str;
    }

    os_ << node.value << "\n";

    if (node.children.empty()) {
      return;
    }

    if (!prefixes.empty()) {
      if (prefixes.back() == "|-") {
        prefixes.back() = "| ";
      } else if (prefixes.back() == "`-") {
        prefixes.back() = "  ";
      }
    }

    for (size_t i = 0; i < node.children.size(); ++i) {
      prefixes.push_back(i + 1 == node.children.size() ? "`-" : "|-");
      print_recursive(*node.children[i], prefixes);
      prefixes.pop_back();
    }
  }

 protected:
  explicit TreePrinter(std::ostream& os)
      : root_(std::make_unique<TextNode>("")),
        nodes_stack_{root_.get()},
        os_(os) {}

  void move_cursor_down() { nodes_stack_.push_back(nullptr); }
  void move_cursor_up() { nodes_stack_.pop_back(); }

  void add_node(std::string_view value) {
    nodes_stack_.pop_back();

    auto node = std::make_unique<TextNode>(std::string(value));
    auto node_ptr = node.get();
    nodes_stack_.back()->children.push_back(std::move(node));

    nodes_stack_.push_back(node_ptr);
  }

  void print() {
    std::vector<std::string> prefixes;
    print_recursive(*root_->children.front(), prefixes);
    nodes_stack_.pop_back();
  }
};
