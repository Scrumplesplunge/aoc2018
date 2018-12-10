#include "puzzles.h"

#include <iostream>
#include <sstream>
#include <iterator>
#include <string_view>
#include <vector>
#include <string>

struct Vector { int x, y; };
struct Point { Vector position, velocity; };

struct BoundingBox {
  // Half-open bounds: min is included, max is excluded.
  Vector min, max;
};

constexpr long area(const BoundingBox& box) {
  return long{box.max.x - box.min.x} * long{box.max.y - box.min.y};
}

constexpr Vector operator+(Vector a, Vector b) {
  return Vector{a.x + b.x, a.y + b.y};
}

constexpr Vector operator-(Vector a, Vector b) {
  return Vector{a.x - b.x, a.y - b.y};
}

constexpr Vector operator*(Vector a, int b) {
  return Vector{a.x * b, a.y * b};
}

constexpr Vector at(Point p, int time) {
  return p.position + p.velocity * time;
}

int svtoi(std::string_view input) {
  const char* begin = input.data();
  char* end = nullptr;
  int result = std::strtol(begin, &end, 10);
  assert(begin != end);
  return result;
}

std::istream& operator>>(std::istream& input, Point& point) {
  std::string line_data;
  if (!std::getline(input, line_data)) return input;
  auto number_after =
      [i = 0, line = std::string_view{line_data}](char c) mutable {
    auto result = line.find(c, i);
    assert(result != std::string::npos);
    i = result + 1;
    return svtoi(line.substr(i));
  };
  point.position.x = number_after('<');
  point.position.y = number_after(',');
  point.velocity.x = number_after('<');
  point.velocity.y = number_after(',');
  return input;
}

BoundingBox Bounds(const std::vector<Point>& points, int time) {
  assert(!points.empty());
  Vector initial = at(points.front(), time);
  BoundingBox bounds{initial, initial};
  for (const Point& point : points) {
    Vector position = at(point, time);
    bounds.min.x = std::min(bounds.min.x, position.x);
    bounds.min.y = std::min(bounds.min.y, position.y);
    bounds.max.x = std::max(bounds.max.x, position.x);
    bounds.max.y = std::max(bounds.max.y, position.y);
  }
  // bounds.max is currently included, it should be excluded.
  bounds.max.x++;
  bounds.max.y++;
  return bounds;
}

int FindAlignmentTime(const std::vector<Point>& points) {
  //for (int time = 0; true; time++) {
  //  if (area(Bounds(points, time)) < area(Bounds(points, time + 1)))
  //    return time;
  //}
  //return -1;
  // Assumption: bounding box will shrink at first, then start growing. The
  // message will appear when the box is at its minimum size.
  long min_time = 0, max_time = 1;
  // Discover an upper bound for the time of the message.
  while (area(Bounds(points, max_time - 1)) > area(Bounds(points, max_time))) {
    min_time = max_time;
    max_time = 2 * max_time;
  }
  while (true) {
    assert(min_time < max_time);
    int mid = min_time + (max_time - min_time) / 2;
    long before = area(Bounds(points, mid - 1));
    long at = area(Bounds(points, mid));
    long after = area(Bounds(points, mid + 1));
    if (before > at && at > after) {
      // Still decreasing but possibly at after.
      min_time = mid + 1;
    } else if (before < at && at < after) {
      // Completely increasing; max is no earlier than mid-1, so max=mid.
      max_time = mid;
    } else {
      // Exact match.
      assert(before > at && at < after);
      return mid;
    }
  }
}

std::string Solve10A() { 
  std::istringstream input{std::string{kPuzzle10}};
  const std::vector<Point> points{std::istream_iterator<Point>{input}, {}};
  int time = FindAlignmentTime(points);
  BoundingBox bounds = Bounds(points, time);
  int width = bounds.max.x - bounds.min.x, height = bounds.max.y - bounds.min.y;
  constexpr int kMaxWidth = 150, kMaxHeight = 10;
  assert(width <= kMaxWidth);
  assert(height <= kMaxHeight);
  std::string result((width + 1) * height + 1, ' ');
  for (int y = 0; y <= height; y++) result[y * (width + 1)] = '\n';
  for (const Point& point : points) {
    Vector position = at(point, time) - bounds.min;
    assert(0 <= position.x && position.x < width);
    assert(0 <= position.y && position.y < height);
    result[1 + position.x + (width + 1) * position.y] = '#';
  }
  return result;
}

int Solve10B() {
  std::istringstream input{std::string{kPuzzle10}};
  const std::vector<Point> points{std::istream_iterator<Point>{input}, {}};
  return FindAlignmentTime(points);
}
