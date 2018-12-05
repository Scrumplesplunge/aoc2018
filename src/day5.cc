#include "day5.h"

#include "puzzles.h"

#include <cctype>
#include <iostream>
#include <iterator>
#include <list>

namespace {

// Uppercase and lowercase ascii differ only by bit 0x20.
constexpr char kLowerCaseBit = 0x20;

bool UnitsReact(char a, char b) { return (a ^ b ^ kLowerCaseBit) == 0; }

// Uncomment debug lines for an output like this (best on *small* toy inputs):
// https://gist.github.com/Scrumplesplunge/40d18bbe5ba065cfdcefaacdde9fdecc
int React(std::string polymer) {
  // std::cout << "\n";  // debug
  while (true) {
    std::size_t n = polymer.size();
    std::size_t gap_size = 0;
    std::size_t i = 0;
    while (i + 1 + gap_size < n) {
      // std::cout << polymer << "\n"  // debug
      //           << std::string(i, ' ') << '^' << std::string(gap_size, ' ')
      //           << "^ " << polymer[i] << polymer[i + 1 + gap_size] << ": ";
      if (UnitsReact(polymer[i], polymer[i + 1 + gap_size])) {
        // std::cout << "reduce\n";  // debug
        // polymer[i] = '_';  // debug
        // polymer[i + 1 + gap_size] = '_';  // debug
        gap_size += 2;
        if (i == 0) {
          std::size_t remaining = n - gap_size;
          auto first = polymer.begin() + gap_size,
               last = first + std::min(gap_size, remaining);
          move(first, last, polymer.begin());
          // fill(first, last, '_');  // debug
          i = gap_size - 1;
        } else {
          i--;
        }
      } else {
        // std::cout << "shift\n";  // debug
        i++;
        if (gap_size) {
          polymer[i] = polymer[i + gap_size];
          // polymer[i + gap_size] = '_';  // debug
        }
      }
      // std::cin.get();  // debug
    }
    if (gap_size == 0) break;
    polymer.resize(polymer.length() - gap_size);
  }
  return polymer.length();
}

}  // namespace

int Solve5A() {
  // We have std::prev because the input ends in a newline.
  return React(std::string{begin(kPuzzle5), std::prev(end(kPuzzle5))});
}

int Solve5B() {
  char best = '?';
  int best_length = kPuzzle5.length();
  for (char c = 'a'; c <= 'z'; c++) {
    std::string polymer;
    polymer.reserve(kPuzzle5.length());
    copy_if(begin(kPuzzle5), std::prev(end(kPuzzle5)),
            inserter(polymer, polymer.end()),
            [c](char c2) { return (c2 | kLowerCaseBit) != c; });
    int length = React(std::move(polymer));
    if (length < best_length) {
      best = c;
      best_length = length;
    }
  }
  return best_length;
}
