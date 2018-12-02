#include "puzzles.h"
#include "timing.h"

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

int main() {
  std::cout << "1A: " << Time(Solve1A) << "\n"
            << "1B: " << Time(Solve1B) << "\n";
}
