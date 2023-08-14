#include "puzzles.h"

#include <array>
#include <cassert>
#include <iterator>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <string_view>

namespace {

struct Rectangle {
  int x, y, width, height;
};

struct Claim {
  int id;
  Rectangle rectangle;
};

int svtoi(std::string_view input) {
  const char* begin = input.data();
  char* end = nullptr;
  int result = std::strtol(begin, &end, 10);
  assert(begin != end);
  return result;
}

std::istream& operator>>(std::istream& input, Claim& claim) {
  std::string line_data;
  if (!std::getline(input, line_data)) return input;
  std::string_view line{line_data};
  claim.id = svtoi(line.substr(1));
  claim.rectangle.x = svtoi(line.substr(line.find('@') + 1));
  claim.rectangle.y = svtoi(line.substr(line.find(',') + 1));
  claim.rectangle.width = svtoi(line.substr(line.find(':') + 1));
  claim.rectangle.height = svtoi(line.substr(line.find('x') + 1));
  return input;
}

}  // namespace

int Solve3A() {
  std::istringstream input{std::string{kPuzzle3}};
  std::vector<Claim> claims{std::istream_iterator<Claim>{input}, {}};
  std::array<std::array<char, 1000>, 1000> fabric = {};
  auto blit = [&fabric](const Rectangle& rectangle) {
    int x_max = rectangle.x + rectangle.width,
        y_max = rectangle.y + rectangle.height;
    for (int y = rectangle.y; y < y_max; y++) {
      for (int x = rectangle.x; x < x_max; x++) {
        fabric[y][x]++;
      }
    }
  };
  for (const Claim& claim : claims) blit(claim.rectangle);
  int overlapping = 0;
  for (const auto& row : fabric) {
    for (char cell : row) {
      if (cell > 1) overlapping++;
    }
  }
  return overlapping;
}

int Solve3B() {
  std::istringstream input{std::string{kPuzzle3}};
  std::vector<Claim> claims{std::istream_iterator<Claim>{input}, {}};
  for (const Claim& claim : claims) {
    auto overlaps = [&](const Claim& other) {
      const Rectangle& a = claim.rectangle;
      const Rectangle& b = other.rectangle;
      bool overlaps_x = !(a.x + a.width <= b.x || b.x + b.width <= a.x);
      bool overlaps_y = !(a.y + a.height <= b.y || b.y + b.height <= a.y);
      return &a != &b && overlaps_x && overlaps_y;
    };
    if (none_of(begin(claims), end(claims), overlaps)) return claim.id;
  }
  return -1;
}
