#include "puzzles.h"
#include "timing.h"

#include <array>
#include <chrono>
#include <iostream>
#include <iterator>
#include <numeric>
#include <set>
#include <sstream>
#include <vector>

int Solve1A() {
  std::istringstream input{std::string{kPuzzle1}};
  return std::reduce(std::istream_iterator<int>{input}, {});
}

int Solve1B() {
  std::istringstream input{std::string{kPuzzle1}};
  const std::vector<int> deltas{std::istream_iterator<int>{input}, {}};
  std::set<int> seen;
  int frequency = 0;
  for (int i = 0; seen.count(frequency) == 0; i++) {
    seen.insert(frequency);
    frequency += deltas[i % deltas.size()];
  }
  return frequency;
}

int Solve2A() {
  std::istringstream input{std::string{kPuzzle2}};
  int count_with_two = 0, count_with_three = 0;
  for (std::string box_id; std::getline(input, box_id);) {
    std::array<int, 26> counts_per_letter = {};
    for (char letter : box_id) {
      assert(std::isalpha(letter));
      counts_per_letter[letter - 'a']++;
    }
    auto has_count = [&](int n) {
      auto begin = std::begin(counts_per_letter),
           end = std::end(counts_per_letter);
      return std::any_of(begin, end, [&](int count) { return count == n; });
    };
    if (has_count(2)) count_with_two++;
    if (has_count(3)) count_with_three++;
  }
  return count_with_two * count_with_three;
}

std::string Solve2B() {
  std::istringstream input{std::string{kPuzzle2}};
  const std::vector<std::string> box_ids{
      std::istream_iterator<std::string>{input}, {}};
  for (const std::string& box_id : box_ids) {
    for (const std::string& other_id : box_ids) {
      assert(box_id.length() == other_id.length());
      int num_differing_letters =
          transform_reduce(begin(box_id), end(box_id), begin(other_id),
                           0, std::plus<>(), std::not_equal_to<>());
      if (num_differing_letters == 1) {
        std::string result = other_id;
        auto [i, j] = std::mismatch(begin(box_id), end(box_id), begin(result));
        result.erase(j);
        return result;
      }
    }
  }
  return "not found";
}

int main() {
  std::cout << "1A: " << Time(Solve1A) << "\n"
            << "1B: " << Time(Solve1B) << "\n"
            << "2A: " << Time(Solve2A) << "\n"
            << "2B: " << Time(Solve2B) << "\n";
}
