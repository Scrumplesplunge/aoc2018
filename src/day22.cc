#include "puzzles.h"

#include "vec2.h"

#include <array>
#include <cassert>
#include <iostream>
#include <limits>
#include <numeric>
#include <queue>
#include <unordered_set>
#include <vector>

namespace {

int svtoi(std::string_view input) {
  const char* begin = input.data();
  char* end = nullptr;
  int result = std::strtol(begin, &end, 10);
  assert(begin != end);
  return result;
}

using Position = vec2<short>;

struct Input { int depth; Position target; };
enum Cell : std::int8_t { kRocky, kWet, kNarrow };
enum Tool : std::int8_t { kTorch, kClimbingGear, kNeither };
struct Configuration { Position position; Tool tool; };

constexpr bool operator==(Configuration a, Configuration b) {
  return a.position == b.position && a.tool == b.tool;
}

Input GetInput() {
  assert(kPuzzle22.substr(0, 7) == "depth: ");
  int depth = svtoi(kPuzzle22.substr(7));
  auto target_label = kPuzzle22.find("target: ");
  assert(target_label != std::string_view::npos);
  int x = svtoi(kPuzzle22.substr(target_label + 8));
  auto comma = kPuzzle22.find(',', target_label);
  int y = svtoi(kPuzzle22.substr(comma + 1));
  assert(0 <= depth && depth < 20183);
  assert(0 < x);
  assert(0 < y);
  return Input{depth, {x, y}};
}

constexpr std::array<Position, 4> AdjacentSquares(Position p) {
  return {{{p.x, p.y - 1}, {p.x, p.y + 1}, {p.x - 1, p.y}, {p.x + 1, p.y}}};
}

constexpr std::array<Tool, 2> CompatibleTools(Cell cell) {
  switch (cell) {
    case kRocky: return {kTorch, kClimbingGear};
    case kWet: return {kClimbingGear, kNeither};
    case kNarrow: return {kTorch, kNeither};
  }
}

constexpr bool Compatible(Cell cell, Tool tool) {
  switch (cell) {
    case kRocky: return tool != kNeither;
    case kWet: return tool != kTorch;
    case kNarrow: return tool != kClimbingGear;
  }
}

}  // namespace

template <>
struct std::hash<Configuration> {
  constexpr std::size_t operator()(Configuration configuration) const {
    return (configuration.position.x * 107) ^
           (configuration.position.y * 5) ^
           configuration.tool;
  }
};

int Solve22A() {
  auto [depth, target] = GetInput();
  std::vector<int> row;
  row.reserve(target.x + 1);
  row.push_back(depth);
  for (int x = 1; x <= target.x; x++) {
    row.push_back((x * 16807 + depth) % 20183);
  }
  auto row_risk = [&] {
    return transform_reduce(
        begin(row), end(row), 0, std::plus<>(), [](auto x) { return x % 3; });
  };
  int risk_total = row_risk();
  for (int y = 1; y <= target.y; y++) {
    row[0] = (y * 48271 + depth) % 20183;
    for (int x = 1; x <= target.x; x++) {
      int left = row[x - 1];  // Already refreshed, so from this row.
      int above = row[x];  // Not refreshed yet, so from the row above.
      if (x == target.x && y == target.y) {
        row[x] = depth;
      } else {
        row[x] = (left * above + depth) % 20183;
      }
    }
    risk_total += row_risk();
  }
  return risk_total;
}

int Solve22B() {
  auto [depth, target] = GetInput();
  // Build the grid.
  int grid_width = std::max(target.x, target.y) + 10;
  int grid_height = std::max(target.x, target.y) + 10;
  assert(grid_width * grid_height < 10'000'000);
  std::vector<short> grid;
  grid.reserve(grid_width * grid_height);
  grid.push_back(depth);
  for (int x = 1; x < grid_width; x++)
    grid.push_back((x * 16807 + depth) % 20183);
  for (int y = 1; y < grid_height; y++) {
    int offset = grid_width * y;
    grid.push_back((y * 48271 + depth) % 20183);
    for (int x = 1; x < grid_width; x++) {
      int left = grid[offset + x - 1];
      int above = grid[offset + x - grid_width];
      if (x == target.x && y == target.y) {
        grid.push_back(depth);
      } else {
        grid.push_back((left * above + depth) % 20183);
      }
    }
  }
  auto cell = [&](int x, int y) { return Cell(grid[y * grid_width + x] % 3); };
  // Search for the cell.
  std::unordered_set<Configuration> explored;
  struct Node { short time, cost; Configuration configuration; };
  auto make_node = [target=target](short time, Configuration configuration) {
    const auto& [pos, tool] = configuration;
    int min_travel_time = abs(target.x - pos.x) + abs(target.y - pos.y);
    int tool_switch_time = tool == kTorch ? 0 : 7;
    short cost = time + min_travel_time + tool_switch_time;
    return Node{time, cost, configuration};
  };
  auto by_cost = [](const Node& a, const Node& b) { return a.cost > b.cost; };
  std::priority_queue<Node, std::vector<Node>, decltype(by_cost)> frontier{
      by_cost};
  frontier.push(make_node(0, Configuration{{0, 0}, kTorch}));
  while (true) {
    assert(!frontier.empty());
    Node node = frontier.top();
    frontier.pop();
    if (node.configuration == Configuration{target, kTorch}) return node.time;
    auto [i, was_inserted] = explored.insert(node.configuration);
    if (!was_inserted) continue;  // Already explored.
    Cell current_cell =
        cell(node.configuration.position.x, node.configuration.position.y);
    for (Position p : AdjacentSquares(node.configuration.position)) {
      if (p.x < 0) continue;
      if (p.y < 0) continue;
      assert(p.x < grid_width);
      assert(p.y < grid_height);
      for (Tool t : CompatibleTools(cell(p.x, p.y))) {
        if (!Compatible(current_cell, t)) continue;  // Can't switch to tool.
        short time = node.time + 1;
        if (t != node.configuration.tool) time += 7;
        frontier.push(make_node(time, {p, t}));
      }
    }
  }
}
