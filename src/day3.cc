#include "day3.h"

#include "puzzles.h"

#include <array>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>

namespace {

struct Rectangle {
  int x, y, width, height;
};

struct Claim {
  int id;
  Rectangle rectangle;
};

const std::regex kElfClaimRegex{
    R"(^#([0-9]+) @ ([0-9]+),([0-9]+): ([0-9]+)x([0-9]+))"};
//       ^ 1        ^ 2      ^ 3       ^ 4      ^ 5

std::istream& operator>>(std::istream& input, Claim& claim) {
  std::string line;
  if (!std::getline(input, line)) return input;
  std::smatch match;
  if (!std::regex_match(line, match, kElfClaimRegex)) {
    input.setstate(std::ios_base::failbit);
    return input;
  }
  claim.id = stoi(match[1]);
  claim.rectangle.x = stoi(match[2]);
  claim.rectangle.y = stoi(match[3]);
  claim.rectangle.width = stoi(match[4]);
  claim.rectangle.height = stoi(match[5]);
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
