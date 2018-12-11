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

class Grid {
 public:
  Grid(int serial_number) {
    grid_ = {};
    for (int y = 1; y <= 300; y++) {
      for (int x = 1; x <= 300; x++) {
        // The cumulative value of this cell is the one up and to the left of
        // it, plus the cumulative value of the column above it and the row to
        // the left of it. However, we don't directly have the latter values so
        // we count the full rectangle from [1, 1] down to the cell above and
        // the same down to the cell to the left and then subtract the now
        // double-counted count down to the cell up and to the left.
        grid_[y][x] = grid_[y - 1][x] + grid_[y][x - 1] - grid_[y - 1][x - 1] +
                      power(x, y, serial_number);
      }
    }
  }

  int block_power(int x, int y, int size) const {
    return grid_[y + size - 1][x + size - 1] -
           grid_[y + size - 1][x - 1] -  // left of this area
           grid_[y - 1][x + size - 1] +  // above this area
           grid_[y - 1][x - 1];        // above and left (double-counted).
  }

 private:
  std::array<std::array<short, 301>, 301> grid_;
};

}  // namespace

std::string Solve11A() {
  const Grid grid{stoi(std::string(kPuzzle11))};
  struct { int x = 1, y = 1; } max_block;
  int max_power = grid.block_power(max_block.x, max_block.y, 3);
  for (int y = 1; y <= 298; y++) {
    for (int x = 1; x <= 298; x++) {
      int current_block_power = grid.block_power(x, y, 3);
      if (current_block_power > max_power) {
        max_power = current_block_power;
        max_block = {x, y};
      }
    }
  }
  return std::to_string(max_block.x) + "," + std::to_string(max_block.y);
}

std::string Solve11B() {
  const Grid grid{stoi(std::string(kPuzzle11))};
  struct { int x = 1, y = 1, size = 1; } max_block;
  int max_power = grid.block_power(max_block.x, max_block.y, max_block.size);
  for (int y = 1; y <= 300; y++) {
    for (int x = 1; x <= 300; x++) {
      for (int size = 1, bound = 300 - std::max(x, y); size <= bound; size++) {
        int power = grid.block_power(x, y, size);
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
