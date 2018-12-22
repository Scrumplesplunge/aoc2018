#include "puzzles.h"

#include <array>
#include <iostream>
#include <numeric>
#include <queue>
#include <set>
#include <vector>

namespace {

int svtoi(std::string_view input) {
  const char* begin = input.data();
  char* end = nullptr;
  int result = std::strtol(begin, &end, 10);
  assert(begin != end);
  return result;
}

struct Position { int x, y; };
struct Input { int depth; Position target; };
enum Cell { kRocky, kWet, kNarrow };
enum Tool { kTorch, kClimbingGear, kNeither };
struct Configuration { Position position; Tool tool; };

constexpr bool operator<(Configuration a, Configuration b) {
  return std::tie(a.position.x, a.position.y, a.tool) <
         std::tie(b.position.x, b.position.y, b.tool);
}

constexpr bool operator==(Configuration a, Configuration b) {
  return a.position.x == b.position.x && a.position.y == b.position.y &&
         a.tool == b.tool;
}

Input GetInput() {
  assert(kPuzzle22.substr(0, 7) == "depth: ");
  int depth = svtoi(kPuzzle22.substr(7));
  auto target_label = kPuzzle22.find("target: ");
  assert(target_label != std::string_view::npos);
  int x = svtoi(kPuzzle22.substr(target_label + 8));
  auto comma = kPuzzle22.find(',', target_label);
  int y = svtoi(kPuzzle22.substr(comma + 1));
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

int Solve22A() {
  auto [depth, target] = GetInput();
  assert(0 <= depth && depth < 20183);
  assert(0 < target.x);
  assert(0 < target.y);
  std::vector<int> row;
  row.reserve(target.x + 1);
  row.push_back(depth);
  for (int x = 1; x <= target.x; x++) {
    row.push_back((x * 16807 + depth) % 20183);
  }
  std::cout << '\n';
  auto row_risk = [&] {
    //for (auto x : row) {
      //std::cout << x << ' ';
      //switch (Cell(x % 3)) {
      //  case Cell::kRocky: std::cout << '.'; break;
      //  case Cell::kWet: std::cout << '='; break;
      //  case Cell::kNarrow: std::cout << '|'; break;
      //}
    //}
    //std::cout << '\n';
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
  // 9041 too high.
  // 7531 too high.
  return risk_total;
}

int Solve22B() {
  auto [depth, target] = GetInput();
  assert(0 <= depth && depth < 20183);
  assert(0 < target.x);
  assert(0 < target.y);
  // Build the grid.
  int grid_width = std::max(target.x, target.y) + 100;
  int grid_height = std::max(target.x, target.y) + 100;
  assert(grid_width * grid_height < 10'000'000);
  std::vector<int> grid;
  grid.reserve(grid_width * grid_height);
  grid.push_back(depth);
  for (int x = 1; x < grid_width; x++) {
    grid.push_back((x * 16807 + depth) % 20183);
  }
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
  std::set<Configuration> explored;
  struct Node {
    Configuration configuration; int time;
    //std::vector<Configuration> path;
  };
  auto cost = [target=target](const Node& node) {
    const auto& [position, tool] = node.configuration;
    const int min_travel_time =
        abs(target.x - position.x) + abs(target.y - position.y);
    const int tool_switch_time = tool == kTorch ? 0 : 7;
    return node.time + min_travel_time + tool_switch_time;
  };
  auto by_cost = [&](const Node& a, const Node& b) {
    return cost(a) > cost(b);
  };
  std::priority_queue<Node, std::vector<Node>, decltype(by_cost)> frontier{
      by_cost};
  frontier.push(Node{Configuration{{0, 0}, kTorch}, 0}); //, {}});
  while (true) {
    assert(!frontier.empty());
    Node node = frontier.top();
    // std::cout << "n{c{x=" << node.configuration.position.x << ",y="
    //           << node.configuration.position.y << "},tool="
    //           << node.configuration.tool << "},time=" << node.time << "}\n";
    frontier.pop();
    if (node.configuration == Configuration{target, kTorch}) {
      //std::cout << "Path:\n";
      //for (const auto& [position, tool] : node.path)
      //  std::cout << "[" << position.x << "," << position.y << "] tool="
      //            << tool << "\n";
      return node.time;
    }
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
        int time = node.time + 1;
        if (t != node.configuration.tool) time += 7;
        Node new_node{{p, t}, time};  //, node.path};
        // new_node.path.push_back(node.configuration);
        frontier.push(new_node);
      }
    }
  }
  // 821 too low
  // 1018 too low
}
