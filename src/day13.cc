#include "puzzles.h"

#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

namespace {

constexpr int kGridWidth = 150;
constexpr int kGridHeight = 150;
using Grid = std::array<std::array<char, kGridWidth>, kGridHeight>;

enum class Direction : unsigned char {
  kUp,
  kRight,
  kDown,
  kLeft,
};

// Incrementing or decrementing a direction is rotating it clockwise or
// counterclockwise respectively.
constexpr Direction operator--(Direction& value, int) {
  Direction old = value;
  value = static_cast<Direction>((static_cast<unsigned char>(value) + 3) % 4);
  return old;
}

constexpr Direction operator++(Direction& value, int) {
  Direction old = value;
  value = static_cast<Direction>((static_cast<unsigned char>(value) + 1) % 4);
  return old;
}

enum class Choice : unsigned char {
  kLeft,
  kStraight,
  kRight,
};

// Incrementing a choice cycles through the options in the order they are
// supposed to occur.
constexpr Choice operator++(Choice& value, int) {
  Choice old = value;
  value = static_cast<Choice>((static_cast<unsigned char>(value) + 1) % 3);
  return old;
}

struct Position { unsigned char x, y; };

// Order positions such that top left < top right < bottom left < bottom right.
constexpr bool operator<(Position a, Position b) {
  return std::tie(a.y, a.x) < std::tie(b.y, b.x);
}

constexpr bool operator==(Position a, Position b) {
  return a.x == b.x && a.y == b.y;
}

struct Cart {
  Position position;
  Direction direction;
  Choice next_choice;
};

// We reserve one cart for representing crashes. This is outside of the range
// where normal carts would be.
constexpr Cart kCrashedCart{
    {kGridWidth, kGridHeight}, Direction::kUp, Choice::kStraight};

// Order carts by their position.
constexpr bool operator<(Cart a, Cart b) { return a.position < b.position; }

constexpr bool operator==(Cart a, Cart b) {
  return a.position == b.position && a.direction == b.direction &&
         a.next_choice == b.next_choice;
}

struct Input {
  Grid grid;
  std::vector<Cart> carts;
};

Input GetInput() {
#ifndef NDEBUG
  // Verify that the input is a 150x150 grid.
  assert(kPuzzle13.length() == (kGridWidth + 1) * kGridHeight);
  for (int i = 0; i < kGridHeight; i++) {
    assert(kPuzzle13[(1 + kGridWidth) * (i + 1) - 1] == '\n');
  }
#endif  // NDEBUG
  Input input;
  for (unsigned char y = 0; y < kGridHeight; y++) {
    int row_offset = (1 + kGridWidth) * y;
    for (unsigned char x = 0; x < kGridWidth; x++) {
      int offset = row_offset + x;
      char cell = kPuzzle13[offset];
      switch (cell) {
        case '^':
          input.carts.push_back(Cart{{x, y}, Direction::kUp, Choice::kLeft});
          input.grid[y][x] = '|';
          break;
        case '>':
          input.carts.push_back(Cart{{x, y}, Direction::kRight, Choice::kLeft});
          input.grid[y][x] = '-';
          break;
        case 'v':
          input.carts.push_back(Cart{{x, y}, Direction::kDown, Choice::kLeft});
          input.grid[y][x] = '|';
          break;
        case '<':
          input.carts.push_back(Cart{{x, y}, Direction::kLeft, Choice::kLeft});
          input.grid[y][x] = '-';
          break;
        default:
          input.grid[y][x] = cell;
      }
    }
  }
  return input;
}

constexpr auto At(Position position) {
  return [=](Cart cart) { return cart.position == position; };
}

constexpr Cart AdvanceCart(const Grid& grid, Cart cart) {
  assert(1 <= cart.position.x && cart.position.x < kGridWidth - 1);
  assert(1 <= cart.position.y && cart.position.y < kGridHeight - 1);
  switch (cart.direction) {
    case Direction::kUp: cart.position.y--; break;
    case Direction::kDown: cart.position.y++; break;
    case Direction::kLeft: cart.position.x--; break;
    case Direction::kRight: cart.position.x++; break;
  }
  switch (grid[cart.position.y][cart.position.x]) {
    case '\\':
      switch (cart.direction) {
        case Direction::kUp: cart.direction = Direction::kLeft; break;
        case Direction::kRight: cart.direction = Direction::kDown; break;
        case Direction::kDown: cart.direction = Direction::kRight; break;
        case Direction::kLeft: cart.direction = Direction::kUp; break;
      }
      break;
    case '/':
      switch (cart.direction) {
        case Direction::kUp: cart.direction = Direction::kRight; break;
        case Direction::kRight: cart.direction = Direction::kUp; break;
        case Direction::kDown: cart.direction = Direction::kLeft; break;
        case Direction::kLeft: cart.direction = Direction::kDown; break;
      }
      break;
    case '+':
      switch (cart.next_choice++) {
        case Choice::kLeft: cart.direction--; break;
        case Choice::kStraight: break;
        case Choice::kRight: cart.direction++; break;
      }
      break;
  }
  return cart;
}

// void ShowCart(const Grid& grid, Cart cart) {
//   constexpr int kContext = 3;
//   int min_x = cart.position.x >= kContext ? cart.position.x - kContext : 0;
//   int max_x = cart.position.x < kGridWidth - kContext
//                   ? cart.position.x + kContext
//                   : kGridWidth - 1;
//   int min_y = cart.position.y >= kContext ? cart.position.y - kContext : 0;
//   int max_y = cart.position.y < kGridHeight - kContext
//                   ? cart.position.y + kContext
//                   : kGridHeight - 1;
//   for (int y = min_y; y <= max_y; y++) {
//     for (int x = min_x; x <= max_x; x++) {
//       if (x == cart.position.x && y == cart.position.y) {
//         std::cout << "\x1b[32;1m";
//         switch (cart.direction) {
//           case Direction::kUp: std::cout << '^'; break;
//           case Direction::kRight: std::cout << '>'; break;
//           case Direction::kDown: std::cout << 'v'; break;
//           case Direction::kLeft: std::cout << '<'; break;
//           default: std::cout << '?'; break;
//         }
//         std::cout << "\x1b[0m";
//       } else {
//         std::cout << grid[y][x];
//       }
//     }
//     std::cout << '\n';
//   }
// }

Position RunUntilCollision(const Grid& grid, std::vector<Cart> carts) {
  while (true) {
    sort(begin(carts), end(carts));
    for (Cart& cart : carts) {
      Cart next = AdvanceCart(grid, cart);
      if (any_of(begin(carts), end(carts), At(next.position)))
        return next.position;
      cart = next;
    }
  }
}

Position LastCartStanding(const Grid& grid, std::vector<Cart> carts) {
  while (true) {
    sort(begin(carts), end(carts));
    for (Cart& cart : carts) {
      if (cart == kCrashedCart) continue;
      Cart next = AdvanceCart(grid, cart);
      auto crash = find_if(begin(carts), end(carts), At(next.position));
      if (crash == end(carts)) {
        cart = next;
      } else {
        cart = kCrashedCart;
        *crash = kCrashedCart;
      }
    }
    carts.erase(remove(begin(carts), end(carts), kCrashedCart), end(carts));
    assert(!carts.empty());
    if (carts.size() == 1) return carts.front().position;
  }
}

}  // namespace

std::string Solve13A() {
  auto [grid, carts] = GetInput();
  Position result = RunUntilCollision(grid, std::move(carts));
  return std::to_string(result.x) + "," + std::to_string(result.y);
}

std::string Solve13B() {
  auto [grid, carts] = GetInput();
  Position result = LastCartStanding(grid, std::move(carts));
  return std::to_string(result.x) + "," + std::to_string(result.y);
}
