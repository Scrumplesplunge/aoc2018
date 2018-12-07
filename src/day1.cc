#include "puzzles.h"

#include <iterator>
#include <iostream>
#include <numeric>
#include <unordered_set>
#include <sstream>
#include <string>
#include <vector>

int Solve1A() {
  std::istringstream input{std::string{kPuzzle1}};
  return reduce(std::istream_iterator<int>{input}, {});
}

int Solve1B() {
  std::istringstream input{std::string{kPuzzle1}};
  const std::vector<int> deltas{std::istream_iterator<int>{input}, {}};
  std::unordered_set<int> seen;
  // Try to find a match in the first iteration.
  int frequency = 0;
  for (int x : deltas) {
    if (!seen.insert(frequency).second) return frequency;
    frequency += x;
  }
  // Since this is a periodic thing, the first conflict must happen in the ones
  // that have already been seen.
  while (true) {
    for (int x : deltas) {
      if (seen.count(frequency)) return frequency;
      frequency += x;
    }
  }
}
