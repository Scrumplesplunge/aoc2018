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
      : buffer_(std::make_unique<int[]>(max_size)), max_size_(max_size) {}

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
    assert(size() < max_size_);
    buffer_[end_] = value;
    end_ = end_ + 1 == max_size_ ? 0 : end_ + 1;
  }

  void push_front(int value) {
    assert(size() < max_size_);
    begin_ = begin_ == 0 ? max_size_ - 1 : begin_ - 1;
    buffer_[begin_] = value;
  }

  int operator[](int i) const {
    assert(0 <= i && i < size());
    return buffer_[index(i)];
  }

  bool empty() const { return begin_ == end_; }

  int front() const {
    assert(!empty());
    return buffer_[begin_];
  }

  int back() const {
    assert(!empty());
    return buffer_[index(size() - 1)];
  }

  void pop_front() {
    assert(size() > 0);
    begin_ = begin_ + 1 == max_size_ ? 0 : begin_ + 1;
  }

  void pop_back() {
    assert(size() > 0);
    end_ = end_ == 0 ? max_size_ - 1 : end_ - 1;
  }

  int size() const {
    int value = end_ - begin_;
    return value >= 0 ? value : max_size_ + value;
  }

 private:
  constexpr int index(int i) const {
    assert(0 <= i && i < max_size_);
    int j = begin_ + i;
    return j < max_size_ ? j : j - max_size_;
  }

  const std::unique_ptr<int[]> buffer_;
  const int max_size_;
  int begin_ = 0;
  int end_ = 0;
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
