#include "puzzles.h"

#include <algorithm>
#include <cassert>
#include <optional>
#include <vector>

namespace {

struct Vector { int x, y, z; };
struct Nanobot { Vector position; int range; };
struct HalfSpace { Vector position, direction; };

struct Space {
  std::vector<HalfSpace> sides;
};

int svtoi(std::string_view input) {
  const char* begin = input.data();
  char* end = nullptr;
  int result = std::strtol(begin, &end, 10);
  assert(begin != end);
  return result;
}

std::vector<Nanobot> GetInput() {
  std::vector<Nanobot> nanobots;
  std::size_t i = 0;
  auto jump_after = [&i](char needle) {
    auto j = kPuzzle23.find(needle, i);
    assert(j != std::string_view::npos);
    i = j + 1;
  };
  auto remaining_input = [&] { return kPuzzle23.substr(i); };
  while (!remaining_input().empty()) {
    jump_after('<');
    auto x = svtoi(remaining_input());
    jump_after(',');
    auto y = svtoi(remaining_input());
    jump_after(',');
    auto z = svtoi(remaining_input());
    jump_after('=');
    auto r = svtoi(remaining_input());
    nanobots.push_back(Nanobot{{x, y, z}, r});
    jump_after('\n');
  }
  return nanobots;
}

auto distance(Vector a, Vector b) {
  return std::abs(a.x - b.x) + std::abs(a.y - b.y) + std::abs(a.z - b.z);
}

}  // namespace

int Solve23A() {
  auto nanobots = GetInput();
  assert(!nanobots.empty());
  auto by_range = [](const Nanobot& a, const Nanobot& b) {
    return a.range < b.range;
  };
  auto strongest_nanobot =
      max_element(begin(nanobots), end(nanobots), by_range);
  auto [position, range] = *strongest_nanobot;
  auto in_range = [position=position, range=range](const Nanobot& n) {
    return distance(position, n.position) < range;
  };
  auto num_in_range = count_if(begin(nanobots), end(nanobots), in_range);
  return num_in_range;
}
