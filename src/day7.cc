#include "puzzles.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <iterator>
#include <queue>
#include <sstream>
#include <string>

namespace {

struct Dependency {
  char before, after;
};

std::istream& operator>>(std::istream& input, Dependency& dependency) {
  std::string line_data;
  if (!std::getline(input, line_data)) return input;
  std::string_view line = line_data;
  assert(line_data.length() == 48);
  assert(line.substr(0, 5) == "Step ");
  assert('A' <= line[5] && line[5] <= 'Z');
  assert(line.substr(6, 30) == " must be finished before step ");
  assert('A' <= line[36] && line[36] <= 'Z');
  assert(line.substr(37) == " can begin.");
  dependency.before = line[5];
  dependency.after = line[36];
  return input;
}

struct WorkEntry {
  char index;
  int finish_time;
};

constexpr bool operator>(WorkEntry a, WorkEntry b) {
  return a.finish_time > b.finish_time;
}

}  // namespace

std::string Solve7A() {
  // dependencies[x][y] is true if x depends on y.
  std::array<std::array<bool, 26>, 26> dependencies = {};
  std::istringstream input{std::string{kPuzzle7}};
  for (std::istream_iterator<Dependency> i{input}, end{}; i != end; i++) {
    dependencies[i->after - 'A'][i->before - 'A'] = true;
  }
  std::array<bool, 26> done = {};
  auto is_ready = [&](auto& target) {
    char index = &target - &dependencies[0];
    return !done[index] &&
           std::none_of(begin(dependencies[index]), end(dependencies[index]),
                        [](bool x) { return x; });
  };
  std::string order;
  order.reserve(26);
  while (true) {
    auto i = std::find_if(begin(dependencies), end(dependencies), is_ready);
    if (i == end(dependencies)) break;
    char index = i - begin(dependencies);
    done[index] = true;
    order.push_back(index + 'A');
    for (auto& target : dependencies) target[index] = false;
  }
  return order;
}

int Solve7B() {
  // dependencies[x][y] is true if x depends on y.
  std::array<std::array<bool, 26>, 26> dependencies = {};
  std::istringstream input{std::string{kPuzzle7}};
  for (std::istream_iterator<Dependency> i{input}, end{}; i != end; i++) {
    dependencies[i->after - 'A'][i->before - 'A'] = true;
  }
  std::array<bool, 26> started = {};
  auto is_ready = [&](auto& target) {
    char index = &target - &dependencies[0];
    return !started[index] &&
           std::none_of(begin(dependencies[index]), end(dependencies[index]),
                        [](bool x) { return x; });
  };
  std::string order;
  std::priority_queue<WorkEntry, std::vector<WorkEntry>, std::greater<>> queue;
  int time = 0;
  while (true) {
    for (char i = 0; i < 26; i++) {
      constexpr int kNumWorkers = 1 + 4;  // You and four elves.
      if (queue.size() == kNumWorkers) break;  // Already busy.
      if (!started[i] && is_ready(dependencies[i])) {
        started[i] = true;
        queue.push(WorkEntry{i, time + 61 + i});
      }
    }
    if (queue.empty()) break;  // No further progress is possible.
    time = queue.top().finish_time;
    while (!queue.empty() && queue.top().finish_time == time) {
      for (auto& target : dependencies) target[queue.top().index] = false; 
      queue.pop();
    }
  }
  return time;
}
