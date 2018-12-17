// Wrong answer: 289 (too low)

#include "puzzles.h"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <iostream>
#include <numeric>
#include <queue>
#include <tuple>
#include <vector>

namespace {

struct BoundingBox { std::int16_t x_min, y_min, x_max, y_max; };
struct Position { std::int16_t x, y; };

enum class Cell : std::uint8_t {
  kSand = '.',
  kClay = '#',
  kFlowingWater = '|',
  kWater = '~',
};

// Adding bounding boxes combines them into a single box that bounds their area.
constexpr BoundingBox operator+(BoundingBox a, BoundingBox b) {
  return BoundingBox{std::min(a.x_min, b.x_min), std::min(a.y_min, b.y_min),
                     std::max(a.x_max, b.x_max), std::max(a.y_max, b.y_max)};
}

std::vector<BoundingBox> GetInput() {
  std::vector<BoundingBox> input;
  input.reserve(2500);
  for (auto offset = kPuzzle17.find('='); offset != std::string_view::npos;
       offset = kPuzzle17.find('=', offset + 1)) {
    bool x_first = kPuzzle17[offset - 1] == 'x';
    std::int16_t a_min = strtol(kPuzzle17.data() + offset + 1, nullptr, 10);
    assert(0 <= a_min && a_min < 2000);
    std::int16_t a_max = a_min;
    offset = kPuzzle17.find('=', offset + 1);
    assert(kPuzzle17[offset - 1] == (x_first ? 'y' : 'x'));
    assert(offset != std::string_view::npos);
    std::int16_t b_min = strtol(kPuzzle17.data() + offset + 1, nullptr, 10);
    assert(0 <= b_min && b_min < 2000);
    offset = kPuzzle17.find("..", offset + 1);
    assert(offset != std::string_view::npos);
    std::int16_t b_max = strtol(kPuzzle17.data() + offset + 2, nullptr, 10);
    assert(0 <= b_max && b_max < 2000);
    assert(b_min <= b_max);
    if (x_first) {
      input.push_back(BoundingBox{a_min, b_min, a_max, b_max});
    } else {
      input.push_back(BoundingBox{b_min, a_min, b_max, a_max});
    }
  }
  return input;
}

constexpr bool IsSupportCell(Cell cell) {
  return cell == Cell::kClay || cell == Cell::kWater;
}

constexpr bool IsWaterCell(Cell cell) {
  return cell == Cell::kFlowingWater || cell == Cell::kWater;
}

std::vector<Cell> PerformFlow() {
  Position spring{500, 0};
  auto input = GetInput();
  auto bounds = reduce(begin(input), end(input), input.front());
  std::cout << "Bounds: [" << bounds.x_min << ", " << bounds.x_max << "] x ["
            << bounds.y_min << ", " << bounds.y_max << "]\n";
  // We need one space either side of any clay to allow water to fall down.
  int width = 3 + bounds.x_max - bounds.x_min;
  int height = 1 + bounds.y_max - bounds.y_min;
  // Move the data to a grid starting at 0,0
  spring.x -= bounds.x_min - 1;  // no need to adjust y, it'd just be discounted.
  for (auto& vein : input) {
    vein.x_min -= bounds.x_min - 1;
    vein.y_min -= bounds.y_min;
    vein.x_max -= bounds.x_min - 1;
    vein.y_max -= bounds.y_min;
  }
  assert(width < 2000);
  assert(height < 2000);
  std::vector<Cell> grid_data(width * height, Cell::kSand);
  auto grid = [&](int x, int y) -> Cell& { return grid_data[y * width + x]; };
  // auto debug = [&](int cx, int cy) {
  //   constexpr int kContext = 10;
  //   int x_min = std::max(0, cx - kContext);
  //   int y_min = std::max(0, cy - kContext);
  //   int x_max = std::min(width, cx + kContext);
  //   int y_max = std::min(height, cy + kContext);
  //   for (int y = y_min; y < y_max; y++) {
  //     const char* first =
  //         reinterpret_cast<const char*>(grid_data.data() + y * width);
  //     if (y == cy) {
  //       std::cout.write(first + x_min, cx - x_min);
  //       std::cout << "\x1b[32m" << first[cx] << "\x1b[0m";
  //       std::cout.write(first + cx - x_min + 1, x_max - cx - 1);
  //     } else {
  //       std::cout.write(first + x_min, x_max - x_min);
  //     }
  //     std::cout << '\n';
  //   }
  //   std::cin.get();
  // };
  // Put the clay onto an image.
  for (auto vein : input) {
    for (int y = vein.y_min; y <= vein.y_max; y++) {
      for (int x = vein.x_min; x <= vein.x_max; x++) grid(x, y) = Cell::kClay;
    }
  }
  grid(spring.x, spring.y) = Cell::kFlowingWater;
  // All flows that have yet to be fully evaluated.
  std::queue<Position> flows;
  flows.push(spring);
  while (!flows.empty()) {
    Position flow = flows.front();
    flows.pop();
    // Follow the flow vertically downwards until it hits something.
    std::int16_t y = flow.y + 1;
    while (y < height && grid(flow.x, y) == Cell::kSand) {
      grid(flow.x, y) = Cell::kFlowingWater;
      y++;
    }
    if (y == height) continue;  // Hit the bottom; this flow is done.
    if (grid(flow.x, y) == Cell::kFlowingWater) continue;  // Already flowing.
    y--;  // Check if water can pool above the surface that it hit.
    for (; 0 <= y; y--) {
      // Water can spread left and right as long as it is supported from below
      // by either kClay or kWater.
      auto x_min = flow.x;
      while (true) {
        if (x_min < 0) break;  // Can't flow any further.
        auto& cell = grid(x_min, y);
        if (IsSupportCell(cell)) break;  // Can't fill this square.
        cell = Cell::kFlowingWater;
        // If there's no support underneath, the water can't extend sideways.
        if (!IsSupportCell(grid(x_min, y + 1))) break;
        x_min--;
        //debug(x_min, y);
      }
      auto x_max = flow.x;
      while (true) {
        if (x_max < 0) break;  // Can't flow any further.
        auto& cell = grid(x_max, y);
        if (IsSupportCell(cell)) break;  // Can't fill this square.
        cell = Cell::kFlowingWater;
        // If there's no support underneath, the water can't extend sideways.
        if (!IsSupportCell(grid(x_max, y + 1))) break;
        x_max++;
        //debug(x_max, y);
      }
      // If the water reached the edge, it flows off and can't pool.
      if (x_min == -1 || x_max == width) break;
      // If the block underneath each end is not a support block, the water
      // flows down instead of pooling.
      bool supported = true;
      if (!IsSupportCell(grid(x_min, y + 1))) {
        supported = false;
        flows.push(Position{static_cast<std::int16_t>(x_min), y});
      }
      if (!IsSupportCell(grid(x_max, y + 1))) {
        supported = false;
        flows.push(Position{static_cast<std::int16_t>(x_max), y});
      }
      if (!supported) break;
      // If the edges are not clay, the water can't pool.
      if (grid(x_min, y) != Cell::kClay || grid(x_max, y) != Cell::kClay) break;
      // Water filled a level, we keep filling.
      for (auto x = x_min + 1; x < x_max; x++) grid(x, y) = Cell::kWater;
      //debug(flow.x, y);
    }
  }
  return grid_data;
}

}  // namespace

int Solve17A() {
  auto grid_data = PerformFlow();
  return count_if(begin(grid_data), end(grid_data), IsWaterCell);
}

int Solve17B() {
  auto grid_data = PerformFlow();
  return count(begin(grid_data), end(grid_data), Cell::kWater);
}
