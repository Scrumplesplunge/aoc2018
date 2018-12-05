#include "day5.h"

#include "puzzles.h"

#include <cctype>
#include <iostream>
#include <iterator>
#include <list>

namespace {

int React(std::list<char> polymer) {
  assert(!polymer.empty());
  auto i = polymer.begin();
  while (std::next(i) != polymer.end()) {
    char i_value = std::tolower(*i);
    bool i_case = std::islower(*i);
    auto j = std::next(i);
    char j_value = std::tolower(*j);
    bool j_case = std::islower(*j);
    if (i_value == j_value && i_case != j_case) {
      polymer.erase(j);
      if (i == polymer.begin()) {
        polymer.erase(i);
        i = polymer.begin();
      } else {
        polymer.erase(i--);
      }
    } else {
      i++;
    }
  }
  return polymer.size();
}

}  // namespace

int Solve5A() {
  // We have std::prev because the input ends in a newline.
  return React(std::list<char>{begin(kPuzzle5), std::prev(end(kPuzzle5))});
}

int Solve5B() {
  char best = '?';
  int best_length = kPuzzle5.length();
  for (char c = 'a'; c <= 'z'; c++) {
    std::list<char> polymer;
    copy_if(begin(kPuzzle5), std::prev(end(kPuzzle5)),
            inserter(polymer, polymer.end()),
            [c](char c2) { return std::tolower(c2) != c; });
    int length = React(std::move(polymer));
    if (length < best_length) {
      best = c;
      best_length = length;
    }
  }
  return best_length;
}
