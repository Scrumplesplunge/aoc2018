#include "day4.h"

#include "puzzles.h"

#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>

namespace {

enum class GuardStarts : int {};
enum class FallsAsleep {};
enum class WakesUp {};

struct DateTime {
  int year, month, day, hour, minute;
};

constexpr bool operator<(const DateTime& left, const DateTime& right) {
  auto [ay, am, ad, ah, amin] = left;
  auto [by, bm, bd, bh, bmin] = right;
  return std::tie(ay, am, ad, ah, amin) < std::tie(by, bm, bd, bh, bmin);
}

struct LogEntry {
  DateTime when;
  std::variant<GuardStarts, FallsAsleep, WakesUp> data;
};

constexpr bool operator<(const LogEntry& left, const LogEntry& right) {
  return left.when < right.when;
}

const std::regex kLogLinePattern{
    R"(\[(\d+)-(\d+)-(\d+) (\d+):(\d+)\] ()"
    //   ^ 1   ^ 2   ^ 3   ^ 4   ^ 5     ^ 6
    R"((Guard #(\d+) begins shift)|(falls asleep)|(wakes up)))",
    // ^ 7     ^ 8                 ^ 9            ^ 10
    std::regex::optimize};

std::istream& operator>>(std::istream& input, LogEntry& entry) {
  std::string line;
  if (!std::getline(input, line)) return input;
  std::smatch match;
  if (!std::regex_match(line, match, kLogLinePattern)) {
    input.setstate(std::ios_base::failbit);
    return input;
  }
  entry.when.year = stoi(match[1]);
  entry.when.month = stoi(match[2]);
  entry.when.day = stoi(match[3]);
  entry.when.hour = stoi(match[4]);
  entry.when.minute = stoi(match[5]);
  if (match.length(7) > 0) {
    entry.data = GuardStarts{stoi(match[8])};
  } else if (match.length(9) > 0) {
    entry.data = FallsAsleep{};
  } else {
    assert(match.length(10) > 0);
    entry.data = WakesUp{};
  }
  return input;
}

struct GuardEntry {
  int total_minutes = 0;
  std::array<char, 60> frequency_per_minute;
};

std::unordered_map<int, GuardEntry> SleepPerGuard() {
  std::istringstream input{std::string{kPuzzle4}};
  std::vector<LogEntry> entries{std::istream_iterator<LogEntry>{input}, {}};
  sort(begin(entries), end(entries));
  assert(std::holds_alternative<GuardStarts>(entries[0].data));
  int current_guard = -1;
  std::unordered_map<int, GuardEntry> sleep_per_guard;
  int asleep_at_min = -1;
  for (const LogEntry& entry : entries) {
    if (const auto* guard = std::get_if<GuardStarts>(&entry.data)) {
      current_guard = static_cast<int>(*guard);
      asleep_at_min = -1;
    } else if (std::holds_alternative<FallsAsleep>(entry.data)) {
      assert(asleep_at_min == -1);
      assert(entry.when.hour == 0);
      asleep_at_min = entry.when.minute;
    } else if (std::holds_alternative<WakesUp>(entry.data)) {
      assert(asleep_at_min != -1);
      assert(entry.when.hour == 0);
      auto& guard_entry = sleep_per_guard[current_guard];
      guard_entry.total_minutes += entry.when.minute - asleep_at_min;
      for (int i = asleep_at_min; i < entry.when.minute; i = (i + 1) % 60)
        guard_entry.frequency_per_minute[i]++;
      asleep_at_min = -1;
    }
  }
  return sleep_per_guard;
}

const auto* MostSleptMinute(const GuardEntry& entry) {
  return std::max_element(begin(entry.frequency_per_minute),
                          end(entry.frequency_per_minute));
}

}  // namespace

int Solve4A() {
  auto sleep_per_guard = SleepPerGuard();
  auto i = max_element(begin(sleep_per_guard), end(sleep_per_guard),
                       [](const auto& a, const auto& b) {
                         return a.second.total_minutes < b.second.total_minutes;
                       });
  const auto& [guard_id, guard_entry] = *i;
  int most_slept_minute =
      MostSleptMinute(guard_entry) - begin(guard_entry.frequency_per_minute);
  return guard_id * most_slept_minute;
}

int Solve4B() {
  auto sleep_per_guard = SleepPerGuard();
  auto i = max_element(begin(sleep_per_guard), end(sleep_per_guard),
                       [&](const auto& a, const auto& b) {
                         return *MostSleptMinute(a.second) <
                                *MostSleptMinute(b.second);
                       });
  auto [guard_id, guard_entry] = *i;
  int minute =
      MostSleptMinute(guard_entry) - begin(guard_entry.frequency_per_minute);
  return guard_id * minute;
}
