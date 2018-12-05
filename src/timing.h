// Timing helpers. Simply wrap a call to your solution function in Time() to get
// accurate timing information for the call. The result is a TimingResult object
// which contains the answer and a time, and it can be output directly:
//
// std::cout << Time(CalculateSixTimesNineOptimized) << "\n";
//
// This will output something like:
//
// 42 in 25ms

#pragma once

#include <chrono>
#include <iostream>
#include <thread>

template <typename T>
struct TimingResult {
  T value;
  std::chrono::nanoseconds time;
};
template <typename T>
TimingResult(T, std::chrono::nanoseconds)->TimingResult<T>;

constexpr int kRepeats = 10;

template <typename T>
std::ostream& operator<<(std::ostream& output, const TimingResult<T>& result) {
  using std::literals::operator "" ns;
  using std::literals::operator "" us;
  using std::literals::operator "" ms;
  using std::literals::operator "" s;

  output << result.value << " in ";
  if (result.time < 10us) {
    return output << (result.time / 1ns) << "ns";
  } else if (result.time < 10ms) {
    return output << (result.time / 1us) << "us";
  } else if (result.time < 10s) {
    return output << (result.time / 1ms) << "ms";
  } else {
    return output << (result.time / 1s) << "s";
  }
}

template <typename F>
auto Time(F&& functor) {
  auto result = functor();
  std::chrono::nanoseconds samples[kRepeats];
  for (int i = 0; i < kRepeats; i++) {
    auto start = std::chrono::steady_clock::now();
    (void) functor();
    auto end = std::chrono::steady_clock::now();
    samples[i] = end - start;
  }
  sort(begin(samples), end(samples));
  return TimingResult{std::move(result), samples[kRepeats / 2]};
}
