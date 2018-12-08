#include "puzzles.h"

#include <cassert>
#include <iostream>
#include <iterator>
#include <numeric>
#include <sstream>
#include <stack>
#include <string>
#include <vector>

namespace {

struct Node {
  std::vector<Node> children;
  std::vector<short> metadata;
};

class Parser {
 public:
  Parser() {
    std::istringstream input{std::string{kPuzzle8}};
    numbers_.assign(std::istream_iterator<short>{input}, {});
  }

  Node ParseNode() {
    assert(offset_ + 2 <= numbers_.size());
    short num_children = numbers_[offset_];
    short num_metadata_entries = numbers_[offset_ + 1];
    offset_ += 2;
    Node output;
    output.children.reserve(num_children);
    for (int i = 0; i < num_children; i++)
      output.children.push_back(ParseNode());
    assert(offset_ + num_metadata_entries <= numbers_.size());
    auto begin = numbers_.data() + offset_, end = begin + num_metadata_entries;
    output.metadata.assign(begin, end);
    offset_ += num_metadata_entries;
    return output;
  }

  bool done() { return offset_ == numbers_.size(); }

 private:
  std::size_t offset_ = 0;
  std::vector<short> numbers_;
};

Node GetInput() {
  Parser parser;
  Node input = parser.ParseNode();
  assert(parser.done());
  return input;
}

int Sum(const Node& node) {
  int x = transform_reduce(
      begin(node.children), end(node.children), 0, std::plus{}, Sum);
  return x + reduce(begin(node.metadata), end(node.metadata));
}

int Value(const Node& node) {
  int n = node.children.size();
  if (n == 0) {
    return reduce(begin(node.metadata), end(node.metadata));
  } else {
    auto child_value = [&](short i) {
      return 1 <= i && i <= n ? Value(node.children[i - 1]) : 0;
    };
    return transform_reduce(begin(node.metadata), end(node.metadata), 0,
                            std::plus{}, child_value);
  }
}

}  // namespace

int Solve8A() { return Sum(GetInput()); }
int Solve8B() { return Value(GetInput()); }
