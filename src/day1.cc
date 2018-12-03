#include "day1.h"

#include "puzzles.h"

#include <iterator>
#include <numeric>
#include <unordered_set>
#include <sstream>
#include <string>
#include <vector>

int Solve1A() {
  std::istringstream input{std::string{kPuzzle1}};
  return std::reduce(std::istream_iterator<int>{input}, {});
}

int Solve1B() {
  std::istringstream input{std::string{kPuzzle1}};
  const std::vector<int> deltas{std::istream_iterator<int>{input}, {}};
  std::unordered_set<int> seen;
  int frequency = 0;
  while (true) {
    for (int x : deltas) {
      if (!seen.insert(frequency).second) return frequency;
      frequency += x;
    }
  }
}
