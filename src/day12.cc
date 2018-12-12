#include "puzzles.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <vector>

namespace {

constexpr int kInitialPots = 99;
constexpr int kNumRules = 32;

struct Input;

class Rules {
 public:
  // pot should be the middle of 5 pots.
  constexpr bool WillGrow(const bool* pot) {
    return mapping_[key(pot)];
  }

 private:
  friend Input GetInput();

  // pot should be the middle of 5 pots.
  constexpr int key(const char* pot) {
    return (pot[-2] == '#') | (pot[-1] == '#') << 1 | (pot[0] == '#') << 2 |
           (pot[1] == '#') << 3 | (pot[2] == '#') << 4;
  }

  // pot should be the middle of 5 pots.
  constexpr int key(const bool* pot) {
    return pot[-2] | pot[-1] << 1 | pot[0] << 2 | pot[1] << 3 | pot[2] << 4;
  }

  std::array<bool, kNumRules> mapping_ = {};
};

struct Input {
  std::array<bool, kInitialPots> pots = {};
  Rules rules;
};

class Pots {
 public:
  Pots(std::int64_t left, std::int64_t right)
      : contents_(right - left), offset_(left), left_(right), right_(left) {}

  const bool& get(std::int64_t index) const {
    assert(offset_ <= index);
    assert(index < offset_ + static_cast<std::int64_t>(contents_.size()));
    return contents_[index - offset_].value;
  }

  void set(std::int64_t index, bool value) {
    if (value) {
      if (index < left_) left_ = index;
      if (right_ <= index) right_ = index + 1;
    }
    contents_[index - offset_].value = value;
  }

  std::int64_t left() const { return left_; }
  std::int64_t right() const { return right_; }

 private:
  struct BoolWrapper { bool value = false; };
  std::vector<BoolWrapper> contents_;
  std::int64_t offset_;
  std::int64_t left_, right_;
};

struct ShiftResult {
  bool is_shifted = false;
  std::int64_t offset = 0;
};

constexpr ShiftResult kNotShifted{};

ShiftResult FindShift(const Pots& a, const Pots& b) {
  std::int64_t range_a = a.right() - a.left();
  std::int64_t range_b = b.right() - b.left();
  if (range_a != range_b) return kNotShifted;
  std::int64_t offset = b.left() - a.left();
  for (std::int64_t i = a.left(); i < a.right(); i++) {
    if (a.get(i) != b.get(i + offset)) return kNotShifted;
  }
  return ShiftResult{true, offset};
}

Input GetInput() {
  Input input;
  // Load the initial space.
  auto initial_begin = kPuzzle12.find(": ");
  assert(initial_begin != std::string_view::npos);
  initial_begin += 2;
  auto initial_end = kPuzzle12.find('\n', initial_begin);
  assert(initial_end != std::string_view::npos);
  assert(initial_end - initial_begin == kInitialPots);
  auto initial = kPuzzle12.substr(initial_begin, initial_end - initial_begin);
  std::transform(begin(initial), end(initial), begin(input.pots),
                 [](char c) { return c == '#'; });
  // Load all the growth rules.
  auto rule_start = kPuzzle12.find("\n\n", initial_end);
  assert(rule_start != std::string_view::npos);
  rule_start += 2;
  int rules_read = 0;
  for (auto i = rule_start, n = kPuzzle12.length() - 1; i < n; i++) {
    auto rule_end = kPuzzle12.find('\n', rule_start);
    if (rule_end == std::string_view::npos) break;
    std::string_view line = kPuzzle12.substr(rule_start, rule_end - rule_start);
    assert(line.length() == 10);
    input.rules.mapping_[input.rules.key(line.data() + 2)] = line[9] == '#';
    rule_start = rule_end + 1;
    rules_read++;
  }
  assert(rules_read == 32);
  return input;
}

std::int64_t GenerationSum(std::int64_t target_generation) {
  Input input = GetInput();
  // Two rows of pots for double buffering. Pot i is at kExpansionBorder + i.
  Pots pots{-5, kInitialPots + 5};
  for (std::int64_t i = 0; i < kInitialPots; i++) pots.set(i, input.pots[i]);
  long generations = 0;
  while (generations < target_generation) {
    generations++;
    Pots previous = std::move(pots);
    std::int64_t begin = previous.left() - 5, end = previous.right() + 5;
    assert(begin < end);
    pots = Pots{begin, end};
    for (std::int64_t i = begin + 2, n = end - 2; i < n; i++) {
      pots.set(i, input.rules.WillGrow(&previous.get(i)));
    }

    if (auto [is_shifted, offset] = FindShift(previous, pots); is_shifted) {
      // This looks the same as the previous iteration, so every subsequent
      // iteration will look identical too aside from a shfit. We can optimize
      // by just jumping straight to the end state.
      std::int64_t final_offset = offset * (target_generation - generations);
      previous = std::move(pots);
      pots =
          Pots{previous.left() + final_offset, previous.right() + final_offset};
      for (std::int64_t i = previous.left(), n = previous.right(); i < n; i++) {
        pots.set(i + final_offset, previous.get(i));
      }
      break;
    }
  }
  // Sum up the pots.
  std::int64_t total = 0;
  for (std::int64_t i = pots.left(), n = pots.right(); i < n; i++) {
    if (pots.get(i)) total += i;
  }
  return total;
}

}  // namespace

std::int64_t Solve12A() { return GenerationSum(20); }
std::int64_t Solve12B() { return GenerationSum(50'000'000'000); }
