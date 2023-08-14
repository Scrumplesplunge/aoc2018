#include "puzzles.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <limits>
#include <numeric>
#include <sstream>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

namespace {

using Id = unsigned char;
using Dimension = short;

struct Coordinate {
  Dimension x, y;
};

enum class Draw : unsigned char {};
using Closest = std::variant<Id, Draw>;

std::istream& operator>>(std::istream& input, Coordinate& coordinate) {
  char comma;
  return input >> coordinate.x >> comma >> coordinate.y;
}

constexpr bool operator<(Coordinate a, Coordinate b) {
  return std::tie(a.x, a.y) < std::tie(b.x, b.y);
}

constexpr Coordinate operator-(Coordinate a, Coordinate b) {
  return Coordinate{static_cast<Dimension>(a.x - b.x),
                    static_cast<Dimension>(a.y - b.y)};
}

constexpr auto abs(Dimension x) { return x < 0 ? -x : x; }

constexpr int Distance(Coordinate a, Coordinate b) {
  return abs(a.x - b.x) + abs(a.y - b.y);
}

struct Input {
  // Input coordinates, offset relative to the min x/y values.
  std::vector<Coordinate> coordinates;
  // Size of the grid (ie. max x and max y, exclusive).
  Coordinate size;
};

Input GetAdjustedCoordinates() {
  std::istringstream input{std::string{kPuzzle6}};
  std::vector<Coordinate> coordinates{
      std::istream_iterator<Coordinate>{input}, {}};
  assert(!coordinates.empty());
  assert(coordinates.size() < std::numeric_limits<Id>::max());
  sort(begin(coordinates), end(coordinates));
  // Find a bounding rectangle for the grid.
  Coordinate min = coordinates.front(), max = min;
  for (Coordinate coordinate : coordinates) {
    min.x = std::min(min.x, coordinate.x);
    min.y = std::min(min.y, coordinate.y);
    max.x = std::max(max.x, coordinate.x);
    max.y = std::max(max.y, coordinate.y);
  }
  // Adjust all coordinates to be relative to the min.
  for (Coordinate& coordinate : coordinates) coordinate = coordinate - min;
  // Need to add 1 to turn the dimensions from inclusive to exclusive bounds.
  Coordinate size{static_cast<Dimension>(1 + max.x - min.x),
                  static_cast<Dimension>(1 + max.y - min.y)};
  return Input{std::move(coordinates), size};
}

}  // namespace

int Solve6A() {
  auto [coordinates, size] = GetAdjustedCoordinates();
  Id n = coordinates.size();
  std::vector<Closest> grid_buffer(size.x * size.y);
  std::vector<int> areas(n);
  // Create a map of owned squares.
  for (Dimension y = 0; y < size.y; y++) {
    for (Dimension x = 0; x < size.x; x++) {
      Coordinate target{x, y};
      Id closest_index = 0;
      int shortest_distance = Distance(coordinates[0], target);
      int distance_count = 1;
      for (Id i = 1; i < n; i++) {
        int distance = Distance(coordinates[i], target);
        if (distance < shortest_distance) {
          closest_index = i;
          shortest_distance = distance;
          distance_count = 1;
        } else if (distance == shortest_distance) {
          distance_count++;
        }
      }
      if (distance_count == 1) {
        areas[closest_index]++;
        grid_buffer[x + size.x * y] = closest_index;
      } else {
        grid_buffer[x + size.x * y] = Draw{};
      }
    }
  }
  // Assumption: any coordinate which is closest to some square on the edge will
  // have infinite area as it will continue to be closest to some squares on the
  // surrounding ring and this applies inductively.
  bool edge[std::numeric_limits<Id>::max() + 1] = {};
  constexpr auto blacklist_edge = [](int offset, int step, int limit,
                                     const Closest* grid, bool* edge) {
    for (int i = offset; i < limit; i += step) {
      if (auto* id = std::get_if<Id>(&grid[i])) edge[*id] = true;
    }
  };
  blacklist_edge(0, 1, size.x, grid_buffer.data(), edge);
  blacklist_edge((size.y - 1) * size.x, 1, size.x, grid_buffer.data(), edge);
  blacklist_edge(0, size.x, size.x * size.y, grid_buffer.data(), edge);
  blacklist_edge(size.x - 1, size.x, size.x * size.y, grid_buffer.data(), edge);
  int max_area = 0;
  for (Id i = 0; i < n; i++) {
    if (!edge[i]) max_area = std::max(max_area, areas[i]);
  }
  return max_area;
}

int Solve6B() {
  auto [coordinates, size] = GetAdjustedCoordinates();
  std::vector<Closest> grid_buffer(size.x * size.y);
  int area = 0;
  for (Dimension y = 0; y < size.y; y++) {
    for (Dimension x = 0; x < size.x; x++) {
      auto compute_distance = [to = Coordinate{x, y}](Coordinate from) {
        return Distance(from, to);
      };
      int cell_rank = transform_reduce(begin(coordinates), end(coordinates), 0,
                                       std::plus<>(), compute_distance);
      if (cell_rank < 10000) {
        // Assumption: the area won't touch the edges of the bounding box.
        assert(0 < x && x < size.x - 1);
        assert(0 < y && y < size.y - 1);
        area++;
      }
    }
  }
  return area;
}
