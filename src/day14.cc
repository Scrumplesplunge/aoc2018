#include "puzzles.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

namespace {

struct State {
  void Step() {
    int score = recipes[elves[0]] + recipes[elves[1]];
    assert(score < 20);
    if (score >= 10) {
      recipes.push_back(1);
      score -= 10;
    }
    recipes.push_back(score);
    for (int i = 0; i < 2; i++) {
      elves[i] += 1 + recipes[elves[i]];
      if (elves[i] >= recipes.size()) elves[i] -= recipes.size();
      assert(elves[i] < recipes.size());
    }
  }

  // Baking the first 10 makes the looping logic simpler.
  std::vector<char> recipes = {3, 7, 1, 0, 1, 0, 1, 2, 4, 5};
  std::uint32_t elves[2] = {6, 3};
};

}  // namespace

std::string Solve14A() {
  int steps = std::stoi(std::string(kPuzzle14));
  std::size_t last = steps + 10;  // Space for the 10 immediately after.
  State state;
  // Reserve space for all recipes of interest plus padding to reduce
  // allocations.
  state.recipes.reserve(last + 1);
  while (state.recipes.size() < last) state.Step();
  // Display the last 10.
  std::string result(10, '?');
  transform(begin(state.recipes) + steps, begin(state.recipes) + last,
            begin(result), [](char recipe) { return '0' + recipe; });
  return result;
}

std::size_t Solve14B() {
  std::string puzzle{kPuzzle14.substr(0, kPuzzle14.length() - 1)};  // remove \n
  for (char& c : puzzle) c -= '0';
  State state;
  std::size_t size_before = state.recipes.size();
  while (true) {
    // If I need more than 100MB I definitely need to consider a better
    // algorithm.
    assert(state.recipes.size() < 100'000'000);
    state.Step();
    std::size_t size_after = state.recipes.size();
    auto first = begin(state.recipes) + size_before - puzzle.length() + 1,
         last = begin(state.recipes) + size_after;
    auto i = search(first, last, begin(puzzle), end(puzzle));
    if (i != last) return i - begin(state.recipes);
    size_before = size_after;
  }
}
