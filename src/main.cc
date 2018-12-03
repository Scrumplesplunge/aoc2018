#include "day1.h"
#include "day2.h"
#include "day3.h"
#include "puzzles.h"
#include "timing.h"

#include <iostream>

int main() {
  std::cout << "1A: " << Time(Solve1A) << "\n"
            << "1B: " << Time(Solve1B) << "\n"
            << "2A: " << Time(Solve2A) << "\n"
            << "2B: " << Time(Solve2B) << "\n"
            << "3A: " << Time(Solve3A) << "\n"
            << "3B: " << Time(Solve3B) << "\n";
}
