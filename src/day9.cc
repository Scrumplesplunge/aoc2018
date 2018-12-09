#include "puzzles.h"

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
    for (int i = 0; i < amount; i++) {
      int f = front();
      pop_front();
      push_back(f);
    }
  }

  void rotate_backward(int amount) {
    assert(amount >= 0);
    for (int i = 0; i < amount; i++) {
      int b = back();
      pop_back();
      push_front(b);
    }
  }

  void push_back(int value) {
    assert(size_ < max_size_);
    buffer_[(position_ + size_) % max_size_] = value;
    size_++;
  }

  void push_front(int value) {
    assert(size_ < max_size_);
    position_--;
    buffer_[(position_ + max_size_) % max_size_] = value;
    size_++;
  }

  int operator[](int index) const {
    assert(0 <= index && index < size_);
    return buffer_[(position_ + index) % max_size_];
  }

  int front() const { return operator[](0); }
  int back() const { return operator[](size_ - 1); }

  void pop_front() {
    assert(size_ > 0);
    position_++;
    size_--;
  }

  void pop_back() {
    assert(size_ > 0);
    size_--;
  }

  int size() const { return size_; }

 private:
  const std::unique_ptr<int[]> buffer_;
  const int max_size_;
  int position_ = 0;
  int size_ = 0;
};

long long Solve(int num_players, int num_marbles) {
  CircularBuffer marbles{num_marbles};
  marbles.push_front(0);
  std::vector<long long> scores(num_players);
  // The active range of marbles is in 0..next_marble with the current marble
  // being the one at the indicated position.
  int player_index = 0;
  for (int player_index = 0, next_marble = 1; next_marble < num_marbles;
       player_index = (player_index + 1) % num_players, next_marble++) {
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
