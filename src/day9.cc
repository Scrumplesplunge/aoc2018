#include "puzzles.h"
#include "timing.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <vector>

namespace {

int svtoi(std::string_view input) {
  const char* begin = input.data();
  char* end = nullptr;
  int result = std::strtol(begin, &end, 10);
  assert(begin != end);
  return result;
}

class CircularBuffer {
 public:
  CircularBuffer(int max_size)
      : buffer_(std::make_unique<int[]>(max_size)),
        begin_(buffer_.get()),
        end_(begin_),
        buffer_end_(begin_ + max_size) {}

  void rotate_forward(int amount) {
    assert(amount >= 0);
    if (empty()) return;
    for (int i = 0; i < amount; i++) {
      int f = front();
      pop_front();
      push_back(f);
    }
  }

  void rotate_backward(int amount) {
    assert(amount >= 0);
    if (empty()) return;
    for (int i = 0; i < amount; i++) {
      int b = back();
      pop_back();
      push_front(b);
    }
  }

  void push_back(int value) {
    assert(size() < max_size());
    *end_ = value;
    end_ = end_ + 1 == buffer_end_ ? buffer_.get() : end_ + 1;
  }

  void push_front(int value) {
    assert(size() < max_size());
    begin_ = begin_ == buffer_.get() ? buffer_end_ - 1 : begin_ - 1;
    *begin_ = value;
  }

  int operator[](int i) const {
    assert(0 <= i && i < size());
    return *index(i);
  }

  bool empty() const { return begin_ == end_; }

  int front() const {
    assert(!empty());
    return *begin_;
  }

  int back() const {
    assert(!empty());
    return *index(size() - 1);
  }

  void pop_front() {
    assert(!empty());
    begin_ = begin_ + 1 == buffer_end_ ? buffer_.get() : begin_ + 1;
  }

  void pop_back() {
    assert(!empty());
    end_ = end_ == buffer_.get() ? buffer_end_ - 1 : end_ - 1;
  }

  int size() const {
    int value = end_ - begin_;
    return value >= 0 ? value : max_size() + value;
  }

  int max_size() const { return buffer_end_ - buffer_.get(); }

 private:
  constexpr int* index(int i) const {
    assert(0 <= i && i < max_size());
    int* j = begin_ + i;
    return j < buffer_end_ ? j : j - max_size();
  }

  const std::unique_ptr<int[]> buffer_;
  int* begin_;
  int* end_;
  int* buffer_end_;
};

long long Solve(int num_players, int num_marbles) {
  CircularBuffer marbles{num_marbles};
  marbles.push_front(0);
  std::vector<long long> scores(num_players);
  // The active range of marbles is in 0..next_marble with the current marble
  // being the one at the indicated position.
  int player_index = 0;
  for (int next_marble = 1; next_marble < num_marbles; next_marble++) {
    if (next_marble % 23 == 0) {
      int n = marbles.size();
      assert(n > 7);
      marbles.rotate_backward(7);
      int front = marbles.front();
      marbles.pop_front();
      scores[player_index] += front + next_marble;
    } else {
      marbles.rotate_forward(2);
      marbles.push_front(next_marble);
    }
    // debug
    //copy(begin(marbles), end(marbles),
    //     std::ostream_iterator<int>{std::cout, " "});
    //std::cout << '\n';
    player_index++;
    if (player_index == num_players) player_index = 0;
  }
  auto i = max_element(begin(scores), end(scores));
  return *i;
}

void Tests() {
  struct TestCase {
    const char* name;
    int num_players, last_marble;
    long long expected_score;
  };
  constexpr TestCase kTestCases[] = {
    {"#1", 10, 1618, 8317},
    {"#2", 13, 7999, 146373},
    {"#3", 17, 1104, 2764},
    {"#4", 21, 6111, 54718},
    {"#5", 30, 5807, 37305},
  };
  std::cout << '\n';
  for (auto test : kTestCases) {
    auto result = Time([&] {
      return Solve(test.num_players, test.last_marble + 1);
    });
    std::cout << test.name << ": " << result << ": "
              << (result.value == test.expected_score ? "success" : "failure")
              << '\n';
  }
}

}  // namespace

long long Solve9A() {
  Tests();
  // N players; last marble is worth M points
  // ^ num_players                   ^ num_marbles
  int num_players = svtoi(kPuzzle9);
  int last_marble = svtoi(kPuzzle9.substr(kPuzzle9.find('h') + 1));
  assert(0 < num_players);
  assert(0 < last_marble);
  int num_marbles = 1 + last_marble;  // 0..n inclusive
  return Solve(num_players, num_marbles);
}

long long Solve9B() {
  int num_players = svtoi(kPuzzle9);
  int last_marble = svtoi(kPuzzle9.substr(kPuzzle9.find('h') + 1));
  assert(0 < num_players);
  assert(0 < last_marble);
  int num_marbles = 1 + 100 * last_marble;
  return Solve(num_players, num_marbles);
}
