#include "puzzles.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

namespace {

enum class GuardStarts : short {};
enum class FallsAsleep : short {};
enum class WakesUp : short {};

struct DateTime {
  unsigned char minute, hour, day, month;
  unsigned int year;
};

constexpr bool operator<(const DateTime& left, const DateTime& right) {
  using ull = unsigned long long;
  ull a = ull{left.year} << 32 | ull{left.month} << 24 | ull{left.day} << 16 |
          ull{left.hour} << 8 | ull{left.minute};
  ull b = ull{right.year} << 32 | ull{right.month} << 24 |
          ull{right.day} << 16 | ull{right.hour} << 8 | ull{right.minute};
  return a < b;
}

struct LogEntry {
  DateTime when;
  std::variant<GuardStarts, FallsAsleep, WakesUp> data;
};

constexpr bool operator<(const LogEntry& left, const LogEntry& right) {
  return left.when < right.when;
}

int svtoi(std::string_view input) {
  const char* begin = input.data();
  char* end = nullptr;
  int result = std::strtol(begin, &end, 10);
  assert(begin != end);
  return result;
}

std::istream& operator>>(std::istream& input, LogEntry& entry) {
  std::string line_data;
  if (!std::getline(input, line_data)) return input;
  std::string_view line{line_data};
  assert(line.length() >= 20);
  assert(line[17] == ']');
  // [YYYY-mm-dd HH:MM] Message
  //  ^1   ^6 ^9 ^12^15 ^19
  entry.when.year = svtoi(line.substr(1));
  entry.when.month = svtoi(line.substr(6));
  entry.when.day = svtoi(line.substr(9));
  entry.when.hour = svtoi(line.substr(12));
  entry.when.minute = svtoi(line.substr(15));
  switch (line[19]) {
    case 'G':
      // [YYYY-mm-dd HH:MM] Guard #N begins shift
      //                           ^26
      entry.data = GuardStarts{static_cast<short>(svtoi(line.substr(26)))};
      break;
    case 'f':
      entry.data = FallsAsleep{};
      break;
    case 'w':
      entry.data = WakesUp{};
      break;
    default:
      assert(false);
  }
  return input;
}

struct GuardEntry {
  int guard_id;
  int total_minutes = 0;
  std::array<char, 60> frequency_per_minute;
};

std::vector<GuardEntry> SleepPerGuard() {
  std::istringstream input{std::string{kPuzzle4}};
  std::vector<LogEntry> entries{std::istream_iterator<LogEntry>{input}, {}};
  sort(begin(entries), end(entries));
  assert(std::holds_alternative<GuardStarts>(entries[0].data));
  std::vector<GuardEntry> sleep_per_guard;
  std::unordered_map<int, std::size_t> guard_map;
  GuardEntry* current_guard = nullptr;
  int asleep_at_min = -1;
  for (const LogEntry& entry : entries) {
    if (const auto* guard = std::get_if<GuardStarts>(&entry.data)) {
      short guard_id = static_cast<short>(*guard);
      auto [i, is_new] = guard_map.emplace(guard_id, sleep_per_guard.size());
      if (is_new) sleep_per_guard.emplace_back().guard_id = guard_id;
      current_guard = &sleep_per_guard[i->second];
      asleep_at_min = -1;
    } else if (std::holds_alternative<FallsAsleep>(entry.data)) {
      assert(asleep_at_min == -1);
      assert(entry.when.hour == 0);
      asleep_at_min = entry.when.minute;
    } else if (std::holds_alternative<WakesUp>(entry.data)) {
      assert(asleep_at_min != -1);
      assert(entry.when.hour == 0);
      current_guard->total_minutes += entry.when.minute - asleep_at_min;
      for (int i = asleep_at_min; i < entry.when.minute; i = (i + 1) % 60)
        current_guard->frequency_per_minute[i]++;
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
                         return a.total_minutes < b.total_minutes;
                       });
  int most_slept_minute = MostSleptMinute(*i) - begin(i->frequency_per_minute);
  return i->guard_id * most_slept_minute;
}

int Solve4B() {
  auto sleep_per_guard = SleepPerGuard();
  auto i = max_element(begin(sleep_per_guard), end(sleep_per_guard),
                       [&](const auto& a, const auto& b) {
                         return *MostSleptMinute(a) < *MostSleptMinute(b);
                       });
  int minute = MostSleptMinute(*i) - begin(i->frequency_per_minute);
  return i->guard_id * minute;
}
