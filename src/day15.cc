// Wrong: 2200 * 95 = 209000
// Wrong: 2200 * 96 = 211200
// Wrong: 2197 * 95 = 208715
// Wrong: 2203 * 95 = 209285
// Right: 2194 * 94 = 206236
//
// Wrong: 12 (11, 13 also wrong)
// Right: 88537 (I'm an idiot; I thought it wanted the damage, not the outcome)
#include "puzzles.h"

#include <array>
#include <cstdint>
#include <iostream>
#include <map>
#include <numeric>
#include <queue>
#include <string_view>
#include <utility>
#include <vector>

namespace {

constexpr int kGridWidth = 32;
constexpr int kGridHeight = 32;
constexpr int kStartingHealth = 200;

enum class UnitType : char { kElf = 'E', kGoblin = 'G' };
struct Position { int x, y; };
using Grid = std::array<std::array<char, kGridWidth>, kGridHeight>;

struct Unit {
  UnitType type;
  Position position;
  std::uint8_t health = kStartingHealth;
};

class State {
 public:
  static State FromInput(int elf_attack_damage);
  void Show();
  void Attack(const std::vector<Position>&);
  bool Move(Unit& unit);
  void Step();
  std::vector<Position> AdjacentEnemies(Position position) const;

  bool done() const { return done_; }
  int outcome() const;

  int num_elves() const { return num_elves_; }
  int num_goblins() const { return num_goblins_; }

 private:
  bool done_ = false;
  int rounds_ = 0;
  Grid grid_;
  std::vector<Unit> units_;
  int num_elves_ = 0;
  int num_goblins_ = 0;
  int elf_attack_damage_ = 3;
};

constexpr bool operator<(Position a, Position b) {
  return std::tie(a.y, a.x) < std::tie(b.y, b.x);
}

constexpr bool operator==(Position a, Position b) {
  return std::tie(a.y, a.x) == std::tie(b.y, b.x);
}

constexpr bool operator<(Unit a, Unit b) { return a.position < b.position; }

void ShowGrid(const Grid& grid) {
  for (const auto& row : grid) {
    for (const auto& cell : row) {
      switch (cell) {
        case '#': std::cout << "\x1b[36m#\x1b[0m"; break;
        case '.': std::cout << "\x1b[30;1m.\x1b[0m"; break;
        case 'E': std::cout << "\x1b[32;1mE\x1b[0m"; break;
        case 'G': std::cout << "\x1b[31;1mG\x1b[0m"; break;
        default: std::cout << "\x1b[35;1m" << cell << "\x1b[0m"; break;
      }
    }
    std::cout << '\n';
  }
}

// Establish reachability of every square in the grid from a given position.
Grid GetDistances(const Grid& grid, Position start) {
  Grid distances = {};
  struct Node { Position position; int distance; };
  std::queue<Node> frontier;
  frontier.push({start, 0});
  while (!frontier.empty()) {
    Node node = frontier.front();
    frontier.pop();
    Position p = node.position;
    int d = node.distance + 1;
    auto add = [&](int x, int y) {
      assert(0 <= x && x < kGridWidth);
      assert(0 <= y && y < kGridHeight);
      if (grid[y][x] != '.') {
        distances[y][x] = 127;
      } else if (distances[y][x]) {
        assert(distances[y][x] <= d);  // Some other path got there first.
      } else {
        distances[y][x] = d;
        frontier.push(Node{{x, y}, d});
      }
    };
    add(p.x, p.y - 1);
    add(p.x, p.y + 1);
    add(p.x - 1, p.y);
    add(p.x + 1, p.y);
  }
  // Mark all unreached cells as max distance.
  for (auto& row : distances) for (auto& cell : row) if (cell == 0) cell = 127;
  distances[start.y][start.x] = 0;
  //Grid debug;
  //for (int y = 0; y < kGridHeight; y++) {
  //  for (int x = 0; x < kGridWidth; x++) {
  //    debug[y][x] =
  //        distances[y][x] == 127 ? grid[y][x] : distances[y][x] % 10 + '0';
  //  }
  //}
  //debug[start.y][start.x] = 'X';
  //std::cout << "Distances from " << start.x << "," << start.y << ":\n";
  //ShowGrid(debug);
  return distances;
}

State State::FromInput(int elf_attack_damage) {
#ifndef NDEBUG
  assert(kPuzzle15.length() == (kGridWidth + 1) * kGridHeight);
  for (int y = 0; y < kGridHeight; y++)
    assert(kPuzzle15[(kGridWidth + 1) * y - 1] == '\n');
#endif // NDEBUG

  State state;
  state.elf_attack_damage_ = elf_attack_damage;
  for (int y = 0; y < kGridHeight; y++) {
    int offset = (kGridWidth + 1) * y;
    for (int x = 0; x < kGridWidth; x++) {
      char c = kPuzzle15[offset + x];
      if (c == 'G') {
        state.num_goblins_++;
        state.units_.push_back(Unit{UnitType::kGoblin, {x, y}});
      } else if (c == 'E') {
        state.num_elves_++;
        state.units_.push_back(Unit{UnitType::kElf, {x, y}});
      } else {
        assert(c == '#' || c == '.');
      }
      state.grid_[y][x] = c;
    }
  }

  return state;
}

void State::Show() { ShowGrid(grid_); }

void State::Attack(const std::vector<Position>& adjacent_enemies) {
  assert(!adjacent_enemies.empty());
  // Attack
  //std::cout << "Attacking ";
  Unit* target = nullptr;
  // Positions are ordered in reading order so the earliest will be picked
  // without special consideration.
  for (const Position& target_position : adjacent_enemies) {
    auto matches_position = [&](const auto& target) {
      return target.health > 0 && target.position == target_position;
    };
    assert(count_if(begin(units_), end(units_), matches_position) == 1);
    auto i = find_if(begin(units_), end(units_), matches_position);
    if (target == nullptr || i->health < target->health) {
      target = &*i;
    }
  }
  assert(target != nullptr);
  //std::cout << target->position.x << "," << target->position.y << ".\n";
  int damage = target->type == UnitType::kElf ? 3 : elf_attack_damage_;
  if (target->health <= damage) {
    // Target killed.
    target->health = 0;
    grid_[target->position.y][target->position.x] = '.';
    auto& target_count =
        target->type == UnitType::kElf ? num_elves_ : num_goblins_;
    target_count--;
  } else {
    // Target wounded.
    target->health -= damage;
  }
}

bool State::Move(Unit& unit) {
  auto [x, y] = unit.position;
  // Pick a target location.
  Grid distances = GetDistances(grid_, unit.position);
  Position best_target = {-1, -1};
  int best_distance = 127;
  for (const auto& target : units_) {
    if (target.health == 0) continue;  // Dead.
    if (target.type == unit.type) continue;  // Same team.
    auto consider = [&](int x, int y) {
      if (grid_[y][x] != '.') return;  // Not somewhere we can go.
      Position p{x, y};
      int distance = distances[y][x];
      if (std::tie(distance, p) < std::tie(best_distance, best_target)) {
        best_distance = distance;
        best_target = p;
      }
    };
    auto [tx, ty] = target.position;
    consider(tx, ty - 1);
    consider(tx - 1, ty);
    consider(tx + 1, ty);
    consider(tx, ty + 1);
  }
  if (best_distance == 127) {
    // std::cout << "Can't move: no available destinations.\n";
    return false;
  }
  assert(best_target.x != -1 && best_target.y != -1);
  // Find the best first step to get to that location.
  distances = GetDistances(grid_, best_target);
  Position next_position = {-1, -1};
  int closest = 127;
  Position candidates[] = {{x, y - 1}, {x - 1, y}, {x + 1, y}, {x, y + 1}};
  for (Position p : candidates) {
    if (grid_[p.y][p.x] != '.') continue;  // Not somewhere we can go.
    int distance = distances[p.y][p.x];
    if (std::tie(distance, p) < std::tie(closest, next_position)) {
      closest = distance;
      next_position = p;
    }
  }
  if (closest == 127) {
    //std::cout << "Can't move: no path.\n";
    return false;
  }
  assert(next_position.x != -1 && next_position.y != -1);
  // Move to the new location.
  //std::cout << "Moving to " << next_position.x << "," << next_position.y
  //          << "\n";
  grid_[y][x] = '.';
  grid_[next_position.y][next_position.x] = static_cast<char>(unit.type);
  unit.position = next_position;
  return true;
}

void State::Step() {
  sort(begin(units_), end(units_));
  for (auto& unit : units_) {
    if (unit.health == 0) continue;
    auto target_count = unit.type == UnitType::kElf ? num_goblins_ : num_elves_;
    if (target_count == 0) {
      done_ = true;
      break;
    }
    assert(1 <= unit.position.x && unit.position.x < kGridWidth - 1);
    assert(1 <= unit.position.y && unit.position.y < kGridHeight - 1);
    assert(static_cast<char>(unit.type) ==
           grid_[unit.position.y][unit.position.x]);
    //std::cout << "Turn for unit at " << x << "," << y << ": ";
    auto adjacent_enemies = AdjacentEnemies(unit.position);
    if (!adjacent_enemies.empty()) {
      Attack(adjacent_enemies);
      continue;
    }
    // Nothing in range. Need to move.
    if (!Move(unit)) continue;  // Can't move.
    adjacent_enemies = AdjacentEnemies(unit.position);
    if (!adjacent_enemies.empty()) Attack(adjacent_enemies);
  }
  constexpr auto is_dead = [](const auto& unit) { return unit.health == 0; };
  units_.erase(remove_if(begin(units_), end(units_), is_dead), end(units_));
  if (!done_) rounds_++;
  sort(begin(units_), end(units_));
#ifndef NDEBUG
  for (const Unit& unit : units_) {
    assert(unit.health > 0);
    //std::cout << (unit.type == UnitType::kElf ? "Elf" : "Goblin") << " "
    //          << unit.position.x << "," << unit.position.y << " has health "
    //          << int{unit.health} << "\n";
  }
#endif  // NDEBUG
}

std::vector<Position> State::AdjacentEnemies(Position position) const {
  auto [x, y] = position;
  assert(0 < x && x < kGridWidth - 1);
  assert(0 < y && y < kGridHeight - 1);
  assert(grid_[y][x] == 'G' || grid_[y][x] == 'E');
  char enemy = grid_[y][x] == 'E' ? 'G' : 'E';
  std::vector<Position> positions;
  positions.reserve(4);
  // Check in reading order.
  if (grid_[y - 1][x] == enemy) positions.push_back({x, y - 1});
  if (grid_[y][x - 1] == enemy) positions.push_back({x - 1, y});
  if (grid_[y][x + 1] == enemy) positions.push_back({x + 1, y});
  if (grid_[y + 1][x] == enemy) positions.push_back({x, y + 1});
  return positions;
}

int State::outcome() const {
  int health = transform_reduce(begin(units_), end(units_), 0, std::plus<>(),
                                [](Unit u) { return u.health; });
  std::cout << "health = " << health << ", rounds = " << rounds_ << "\n";
  return health * rounds_;
}

bool ElfVictoryWith(int damage) {
  auto state = State::FromInput(damage);
  int original_num_elves = state.num_elves();
  int original_num_goblins = state.num_goblins();
  while (!state.done()) state.Step();
  std::cout << "damage = " << damage << ": "
            << (original_num_elves - state.num_elves()) << " elf deaths vs "
            << (original_num_goblins - state.num_goblins())
            << " goblin deaths.\n";
  return state.num_elves() == original_num_elves;
}

}  // namespace

int Solve15A() {
  auto state = State::FromInput(3);
  while (!state.done()) state.Step();
  return state.outcome();
}

int Solve15B() {
  int min_damage = 4, max_damage = 200;
  while (min_damage != max_damage) {
    int damage = min_damage + (max_damage - min_damage) / 2;
    if (ElfVictoryWith(damage)) {
      // Elves win with this amount, so it might be the right amount.
      max_damage = damage;
    } else {
      // At least one elf died, so more damage is needed.
      min_damage = damage + 1;
    }
  }
  auto state = State::FromInput(min_damage);
  while (!state.done()) state.Step();
  return state.outcome();
}
