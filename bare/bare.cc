#include "1.h"

// Since we aren't including anything, we need some type aliases. Naturally,
// these are only correct for some platforms, my own included.
using size_t = unsigned long;
using ssize_t = long;

// This is the only external thing we will use: a single call to write() to
// display the result of the calculations.
extern "C" ssize_t write(int fd, const void* buffer, size_t count);

constexpr bool IsWhitespace(char c) { return c == ' ' || c == '\n'; }
constexpr bool IsDigit(char c) { return '0' <= c && c <= '9'; }

class Reader {
 public:
  constexpr Reader(const char* begin, const char* end)
      : begin_(begin), position_(begin), end_(end) {}

  template <size_t size>
  constexpr Reader(const char (&data)[size])
      : begin_(data), position_(data), end_(data + size) {}

  constexpr bool empty() const { return position_ == end_; }

  constexpr void SkipWhitespace() {
    while (position_ != end_ && IsWhitespace(*position_)) position_++;
  }

  constexpr bool ReadInt(int* value) {
    SkipWhitespace();
    if (position_ == end_) return false;
    bool negative = false;
    if (*position_ == '-' || *position_ == '+') {
      negative = (*position_ == '-');
      position_++;
    }
    if (position_ == end_ || !IsDigit(*position_)) return false;
    *value = 0;
    while (position_ != end_ && IsDigit(*position_)) {
      *value = 10 * (*value) + (*position_ - '0');
      position_++;
    }
    if (negative) *value = -*value;
    return true;
  }

 private:
  const char* begin_;
  const char* position_;
  const char* end_;
};

class Writer {
 public:
  constexpr Writer(char* begin, char* end)
      : begin_(begin), position_(begin), end_(end) {}

  template <size_t size>
  constexpr Writer(char (&data)[size])
      : begin_(data), position_(data), end_(data + size) {}

  constexpr bool full() const { return position_ == end_; }

  constexpr bool Write(const char* c_string) {
    size_t size = 0;
    while (c_string[size]) size++;
    if (position_ + size > end_) return false;
    for (size_t i = 0; i < size; i++) position_[i] = c_string[i];
    position_ += size;
    return true;
  }

  constexpr bool WriteInt(int value) {
    // Build the number backwards in a temporary buffer, since we want to start
    // with the least significant digit.
    bool negative = value < 0;
    if (negative) value = -value;
    char temp[16] = {};
    int i = 16;
    while (value != 0) {
      temp[--i] = (value % 10) + '0';
      value /= 10;
    }
    int length = 16 - i;
    if (negative) length++;
    if (position_ + length > end_) return false;
    // We now know the number fits.
    if (negative) {
      *position_ = '-';
      position_++;
    }
    while (i < 16) {
      *position_ = temp[i];
      position_++;
      i++;
    }
    return true;
  }

  constexpr const char* data() const { return begin_; }
  constexpr size_t size() const { return position_ - begin_; }

 private:
  char* begin_;
  char* position_;
  char* end_;
};

constexpr int Solve1A() {
  Reader reader(kPuzzleInput);
  int x = 0, total = 0;
  while (reader.ReadInt(&x)) total += x;
  return total;
}

// This part looks awkward. In order to guarantee that things happen at compile
// time, we need to evaluate them in a constexpr context. By having a type like
// this with the work in the constructor and the output in the object, we can
// construct the object in a constexpr context to force the calculations to
// happen and then inspect the state in a non-constexpr context to make use of
// it.
class Output {
 public:
  constexpr Output() {
    int solution = Solve1A();
    Writer writer{output_};
    writer.Write("Solve1A: ");
    writer.WriteInt(solution);
    writer.Write("\n");
    size_ = writer.size();
  }

  constexpr const char* data() const { return output_; }
  constexpr size_t size() const { return size_; }

 private:
  char output_[128] = {};
  size_t size_ = 0;
};

constexpr Output output = {};

int main() {
  constexpr const char* data = output.data();
  constexpr size_t size = output.size();
  write(1, data, size);
}
