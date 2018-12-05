#include "day2.h"

#include "puzzles.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <iterator>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

int Solve2A() {
  std::istringstream input{std::string{kPuzzle2}};
  int count_with_two = 0, count_with_three = 0;
  for (std::string box_id; std::getline(input, box_id);) {
    std::array<int, 26> counts_per_letter = {};
    for (char letter : box_id) {
      assert(std::islower(letter));
      counts_per_letter[letter - 'a']++;
    }
    auto has_count = [&](int n) {
      return std::any_of(begin(counts_per_letter), end(counts_per_letter),
                         [&](int count) { return count == n; });
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
          transform_reduce(begin(box_id), end(box_id), begin(other_id), 0,
                           std::plus<>(), std::not_equal_to<>());
      if (num_differing_letters == 1) {
        std::string result = other_id;
        auto [i, j] = mismatch(begin(box_id), end(box_id), begin(result));
        result.erase(j);
        return result;
      }
    }
  }
  return "not found";
}
