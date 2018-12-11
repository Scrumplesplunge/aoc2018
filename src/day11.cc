#include "puzzles.h"

#include <array>
#include <iostream>
#include <string>

namespace {

int power(int x, int y, int serial_number) {
  int rack_id = x + 10;
  int power = rack_id * y;
  power += serial_number;
  power *= rack_id;
  power = power / 100 % 10;
  power -= 5;
  return power;
}

int block_power(int x, int y, int serial_number) {
  int total = 0;
  for (int dy = 0; dy < 3; dy++) {
    for (int dx = 0; dx < 3; dx++) {
      total += power(x + dx, y + dy, serial_number);
    }
  }
  return total;
}

}  // namespace

std::string Solve11A() {
  const int serial_number = stoi(std::string(kPuzzle11));
  struct { int x = 1, y = 1; } max_block;
  int max_power = block_power(max_block.x, max_block.y, serial_number);
  for (int y = 1; y <= 298; y++) {
    for (int x = 1; x <= 298; x++) {
      int current_block_power = block_power(x, y, serial_number);
      if (current_block_power > max_power) {
        max_power = current_block_power;
        max_block = {x, y};
      }
    }
  }
  return std::to_string(max_block.x) + "," + std::to_string(max_block.y);
}

std::string Solve11B() {
  const int serial_number = stoi(std::string(kPuzzle11));
  // The grid cell [i, j] will contain a cumulative sum of all cells in the area
  // with lower bounds [1, 1] and upper bounds [i + 1, j + 1] (inclusive).
  std::array<std::array<int, 300>, 300> grid;
  grid[0][0] = power(1, 1, serial_number);
  for (int x = 1; x < 300; x++) {
    grid[0][x] = grid[0][x - 1] + power(1, 1, serial_number);
  }
  for (int y = 1; y < 300; y++) {
    grid[y][0] = grid[y - 1][0] + power(1, y + 1, serial_number);
    for (int x = 1; x < 300; x++) {
      // The cumulative value of this cell is the one up and to the left of it,
      // plus the cumulative value of the column above it and the row to the
      // left of it. However, we don't directly have the latter values so we
      // count the full rectangle from [1, 1] down to the cell above and the
      // same down to the cell to the left and then subtract the now
      // double-counted count down to the cell up and to the left.
      grid[y][x] = grid[y - 1][x] + grid[y][x - 1] - grid[y - 1][x - 1] +
                   power(x, y, serial_number);
    }
  }
  struct { int x = 1, y = 1, size = 1; } max_block;
  int max_power = grid[0][0];
  for (int y = 0; y < 300; y++) {
    for (int x = 0; x < 300; x++) {
      for (int size = 1, bound = 300 - std::max(x, y); size <= bound; size++) {
        int power = grid[y + size - 1][x + size - 1];
        if (0 < x) power -= grid[y + size - 1][x - 1];  // left of this area
        if (0 < y) power -= grid[y - 1][x + size - 1];  // above this area
        if (0 < x && 0 < y) power += grid[y - 1][x - 1];  // above and left
        if (max_power < power) {
          max_power = power;
          max_block = {x, y, size};
        }
      }
    }
  }
  return std::to_string(max_block.x) + "," + std::to_string(max_block.y) + "," +
         std::to_string(max_block.size);
}
