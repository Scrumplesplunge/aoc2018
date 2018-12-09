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
    for (int i = 0; i < amount; i++) push_back(pop_front());
  }

  void rotate_backward(int amount) {
    assert(amount >= 0);
    for (int i = 0; i < amount; i++) push_front(pop_back());
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

  bool empty() const { return begin_ == end_; }

  int pop_front() {
    assert(!empty());
    int result = *begin_;
    begin_ = begin_ + 1 == buffer_end_ ? buffer_.get() : begin_ + 1;
    return result;
  }

  int pop_back() {
    assert(!empty());
    end_ = end_ == buffer_.get() ? buffer_end_ - 1 : end_ - 1;
    return *end_;
  }

  int size() const {
    int value = end_ - begin_;
    return value >= 0 ? value : max_size() + value;
  }

  int max_size() const { return buffer_end_ - buffer_.get(); }

 private:
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
  int player_index = 0, remainder = 1;
  for (int next_marble = 1; next_marble < num_marbles;
       next_marble++, player_index++, remainder++) {
    if (player_index == num_players) player_index = 0;
    if (remainder == 23) {
      remainder = 0;
      marbles.rotate_backward(7);
      scores[player_index] += marbles.pop_front() + next_marble;
    } else {
      marbles.rotate_forward(2);
      marbles.push_front(next_marble);
    }
  }
  auto i = max_element(begin(scores), end(scores));
  return *i;
}

}  // namespace

long long Solve9A() {
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
