#include "day5.h"

#include "puzzles.h"

#include <cctype>
#include <iostream>
#include <iterator>
#include <list>

namespace {

// The puzzle input ends with a newline. We don't want that. Remove it.
const std::string_view kTrimmedPuzzle = kPuzzle5.substr(0, kPuzzle5.size() - 1);

// Uppercase and lowercase ascii differ only by bit 0x20.
constexpr char kLowerCaseBit = 0x20;

bool UnitsReact(char a, char b) { return (a ^ b ^ kLowerCaseBit) == 0; }

int React(std::string polymer) {
  std::size_t n = polymer.size();
  std::size_t gap_size = 0;
  std::size_t i = 0;
  while (i + 1 + gap_size < n) {
    if (UnitsReact(polymer[i], polymer[i + 1 + gap_size])) {
      // Two things cancelled. The gap in the buffer widens.
      gap_size += 2;
      if (i == 0) {
        // There is nothing to the left still in need of checking so the gap
        // moves forwards.
        polymer[0] = polymer[gap_size];
      } else {
        i--;
      }
    } else {
      i++;
      if (gap_size) {
        // Advance the gap by moving the first thing on the right over to the
        // left.
        polymer[i] = polymer[i + gap_size];
      }
    }
  }
  assert(i + 1 + gap_size == n);
  return n - gap_size;
}

}  // namespace

int Solve5A() {
  // We have std::prev because the input ends in a newline.
  return React(std::string{kTrimmedPuzzle});
}

int Solve5B() {
  char best = '?';
  int best_length = kPuzzle5.length();
  for (char c = 'a'; c <= 'z'; c++) {
    std::string polymer{kTrimmedPuzzle};
    auto i = remove_if(begin(polymer), end(polymer), [c](char c2) {
      return (c2 | kLowerCaseBit) == c;
    });
    polymer.erase(i, end(polymer));
    int length = React(move(polymer));
    if (length < best_length) {
      best = c;
      best_length = length;
    }
  }
  return best_length;
}
