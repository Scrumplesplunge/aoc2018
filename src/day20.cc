#include "puzzles.h"

#include <iostream>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <vector>

namespace {

enum class Cell {
  kFloor = '.',
  kWall = '#',
  kHorizontalDoor = '-',
  kVerticalDoor = '|',
};

using Dimension = int;
struct Position { Dimension x, y; };

constexpr bool operator<(Position a, Position b) {
  return std::tie(a.y, a.x) < std::tie(b.y, b.x);
}

constexpr bool operator==(Position a, Position b) {
  return a.x == b.x && a.y == b.y;
}

struct Hash {
  constexpr auto operator()(Position p) const {
    return (p.x * 19) ^ (p.y * 37);
  }
};

using Grid = std::unordered_map<Position, Cell, Hash>;

constexpr bool IsDirection(char c) {
  return c == 'N' || c == 'E' || c == 'S' || c == 'W';
}

constexpr auto Go(char direction, Grid* grid) {
  assert(IsDirection(direction));
  int dx = 0, dy = 0;
  Cell door = Cell::kFloor;
  switch (direction) {
    case 'N': dx = 0, dy = -1; door = Cell::kHorizontalDoor; break;
    case 'E': dx = 1, dy = 0; door = Cell::kVerticalDoor; break;
    case 'S': dx = 0, dy = 1; door = Cell::kHorizontalDoor; break;
    case 'W': dx = -1, dy = 0; door = Cell::kVerticalDoor; break;
  }
  return [grid, dx, dy, door](Position& p) {
    (*grid)[{p.x + dx, p.y + dy}] = door;
    p.x += 2 * dx;
    p.y += 2 * dy;
  };
}

// Build the grid by walking along the paths described by the regex.
struct WalkResult {
  std::vector<Position> destinations;
  std::string_view remaining_pattern;
};

WalkResult Walk(std::string_view path_pattern,
                const std::vector<Position>& start, Grid* grid) {
  std::size_t i = 0;
  std::size_t n = path_pattern.length();
  std::vector<Position> positions = start;
  while (i < n) {
    char lookahead = path_pattern[i];
    if (lookahead == '$' || lookahead == '|' || lookahead == ')') break;
    if (IsDirection(lookahead)) {
      // Walk along the one available path from each location.
      auto j = path_pattern.find_first_not_of("NESW", i);
      assert(j != std::string_view::npos);
      for (char direction : path_pattern.substr(i, j - i)) {
        for_each(begin(positions), end(positions), Go(direction, grid));
      }
      i = j;
    } else if (lookahead == '(') {
      // Need to consider each option.
      std::vector<Position> new_positions;
      do {
        i++;
        auto temp = Walk(path_pattern.substr(i), positions, grid);
        new_positions.insert(end(new_positions), begin(temp.destinations),
                             end(temp.destinations));
        std::size_t i2 = temp.remaining_pattern.data() - path_pattern.data();
        assert(i2 < n);
        assert(path_pattern[i2] == '|' || path_pattern[i2] == ')');
        i = i2;
      } while (path_pattern[i] != ')');
      sort(begin(new_positions), end(new_positions));
      auto j = unique(begin(new_positions), end(new_positions));
      new_positions.erase(j, end(new_positions));
      swap(positions, new_positions);
      i++;
    }
  }
  return WalkResult{std::move(positions), path_pattern.substr(i)};
}

struct MeasureResult {
  int longest_path;
  int num_long_paths;
};

MeasureResult MeasurePaths(const Grid& grid, Position start) {
  // Look up a cell in the grid. If it is not found, it is either floor or wall
  // depending solely upon whether the coordinate lines up with a cell or a wall
  // slot in the grid.
  auto cell = [&](int x, int y) {
    auto i = grid.find({x, y});
    if (i != grid.end()) return i->second;
    bool is_wall = x % 2 || y % 2;
    return is_wall ? Cell::kWall : Cell::kFloor;
  };
  struct Node { Position position; int distance; };
  int num_long_paths = 0;
  int furthest_distance = 0;
  std::unordered_set<Position, Hash> visited;
  std::queue<Node> positions;
  positions.push({start, 0});
  while (!positions.empty()) {
    auto [p, distance] = positions.front();
    positions.pop();
    auto [_, not_visited] = visited.insert(p);
    if (not_visited) {
      if (distance > furthest_distance) {
        furthest_distance = distance;
      }
      if (distance >= 1000) num_long_paths++;
      if (cell(p.x, p.y - 1) == Cell::kHorizontalDoor)
        positions.push({{p.x, p.y - 2}, distance + 1});
      if (cell(p.x + 1, p.y) == Cell::kVerticalDoor)
        positions.push({{p.x + 2, p.y}, distance + 1});
      if (cell(p.x, p.y + 1) == Cell::kHorizontalDoor)
        positions.push({{p.x, p.y + 2}, distance + 1});
      if (cell(p.x - 1, p.y) == Cell::kVerticalDoor)
        positions.push({{p.x - 2, p.y}, distance + 1});
    }
  }
  return MeasureResult{furthest_distance, num_long_paths};
}

}  // namespace

int Solve20A() {
  // Remove the ^ and also the trailing \n (but not the $).
  auto pattern = kPuzzle20.substr(1, kPuzzle20.length() - 2);
  Grid grid;
  auto result = Walk(pattern, {{0, 0}}, &grid);
  assert(result.remaining_pattern == "$");
  return MeasurePaths(grid, {0, 0}).longest_path;
}

int Solve20B() {
  // Remove the ^ and also the trailing \n (but not the $).
  auto pattern = kPuzzle20.substr(1, kPuzzle20.length() - 2);
  Grid grid;
  auto result = Walk(pattern, {{0, 0}}, &grid);
  assert(result.remaining_pattern == "$");
  return MeasurePaths(grid, {0, 0}).num_long_paths;
}
