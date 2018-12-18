#include "puzzles.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <vector>

namespace {

enum class Cell : char { kOpen = '.', kTrees = '|', kLumberYard = '#' };

constexpr int kGridWidth = 50;
constexpr int kGridHeight = 50;
using Grid = std::array<std::array<Cell, kGridWidth>, kGridHeight>;

Grid GetInput() {
#ifndef NDEBUG
  assert(kPuzzle18.length() == (kGridWidth + 1) * kGridHeight);
  for (int y = 0; y < kGridHeight; y++) {
    assert(kPuzzle18[(1 + kGridWidth) * y - 1] == '\n');
  }
#endif  // NDEBUG

  Grid grid;
  for (int y = 0; y < kGridHeight; y++) {
    const char* first = kPuzzle18.data() + (1 + kGridWidth) * y;
    const char* last = first + kGridWidth;
    std::copy(first, last, reinterpret_cast<char*>(grid[y].data()));
  }
  return grid;
}

int CountAdjacent(const Grid& grid, int cx, int cy, Cell target) {
  int count = 0;
  // Half open bounds - min inclusive, max exclusive.
  int x_min = std::max(0, cx - 1), x_max = std::min(kGridWidth, cx + 2);
  int y_min = std::max(0, cy - 1), y_max = std::min(kGridHeight, cy + 2);
  for (int y = y_min; y < y_max; y++) {
    for (int x = x_min; x < x_max; x++) {
      if (x == cx && y == cy) continue;
      if (grid[y][x] == target) count++;
    }
  }
  return count;
}

void Step(const Grid& before, Grid& after) {
  for (int y = 0; y < kGridHeight; y++) {
    for (int x = 0; x < kGridWidth; x++) {
      switch (before[y][x]) {
        case Cell::kOpen:
          after[y][x] = CountAdjacent(before, x, y, Cell::kTrees) >= 3
                            ? Cell::kTrees
                            : Cell::kOpen;
          break;
        case Cell::kTrees:
          after[y][x] = CountAdjacent(before, x, y, Cell::kLumberYard) >= 3
                            ? Cell::kLumberYard
                            : Cell::kTrees;
          break;
        case Cell::kLumberYard:
          after[y][x] = CountAdjacent(before, x, y, Cell::kLumberYard) >= 1 &&
                                CountAdjacent(before, x, y, Cell::kTrees) >= 1
                            ? Cell::kLumberYard
                            : Cell::kOpen;
          break;
      }
    }
  }
}

int Value(const Grid& grid) {
  const Cell* first = &grid[0][0];
  const Cell* last = &grid[kGridHeight][0];
  int num_trees = std::count(first, last, Cell::kTrees);
  int num_lumber_yards = std::count(first, last, Cell::kLumberYard);
  return num_trees * num_lumber_yards;
}

}  // namespace

int Solve18A() {
  Grid grids[2] = {GetInput(), {}};
  for (int i = 0; i < 10; i++) Step(grids[i % 2], grids[(i + 1) % 2]);
  const Grid& result = grids[10 % 2];
  return Value(result);
}

int Solve18B() {
  std::vector<Grid> previous;
  Grid grids[2] = {GetInput(), {}};
  constexpr int kMaxSearchSize = 1000;  // How long to search for a cycle.
  for (int i = 0; i < kMaxSearchSize; i++) {
    const Grid& before = grids[i % 2];
    Grid& after = grids[(i + 1) % 2];
    Step(before, after);
    auto j = find(begin(previous), end(previous), after);
    if (j == end(previous)) {
      previous.push_back(after);
    } else {
      // This state has been seen before. We can guess generation 1'000'000'000.
      // We are at generation i and we are at the start of a cycle. The cycle
      // will repeat indefinitely, so we just need to work out what stage it
      // will be at (1'000'000'000 - (i + 1)) minutes from now.
      const int last_seen_generation = j - begin(previous);
      const int phase = i - last_seen_generation;
      const int remaining_generations = 1'000'000'000 - (i + 1);
      int offset = remaining_generations % phase;
      const Grid& final_grid = previous[last_seen_generation + offset];
      return Value(final_grid);
    }
  }
  assert(false);  // Not found.
  return -1;
}
